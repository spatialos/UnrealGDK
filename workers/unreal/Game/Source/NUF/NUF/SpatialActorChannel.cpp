// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"
#include "EntityRegistry.h"
#include "Engine/DemoNetDriver.h"
#include "Net/DataBunch.h"
#include "Net/NetworkProfiler.h"
#include "Commander.h"
#include "SpatialNetConnection.h"
#include "SpatialOS.h"
#include "SpatialInterop.h"

DEFINE_LOG_CATEGORY(LogSpatialOSActorChannel);

namespace
{
//This is a bookkeeping function that is similar to the one in RepLayout.cpp, modified for our needs (e.g. no NaKs)
// We can't use the one in RepLayout.cpp because it's private and it cannot account for our approach.
// In this function, we poll for any changes in Unreal properties compared to the last time we replicated this actor.
void UpdateChangelistHistory(FRepState * RepState)
{
	check(RepState->HistoryEnd >= RepState->HistoryStart);

	const int32 HistoryCount = RepState->HistoryEnd - RepState->HistoryStart;
	check(HistoryCount < FRepState::MAX_CHANGE_HISTORY);

	for (int32 i = RepState->HistoryStart; i < RepState->HistoryEnd; i++)
	{
		const int32 HistoryIndex = i % FRepState::MAX_CHANGE_HISTORY;

		FRepChangedHistory & HistoryItem = RepState->ChangeHistory[HistoryIndex];

		check(HistoryItem.Changed.Num() > 0);		// All active history items should contain a change list

		HistoryItem.Changed.Empty();
		HistoryItem.OutPacketIdRange = FPacketIdRange();
		RepState->HistoryStart++;
	}

	// Remove any tiling in the history markers to keep them from wrapping over time
	const int32 NewHistoryCount = RepState->HistoryEnd - RepState->HistoryStart;

	check(NewHistoryCount <= FRepState::MAX_CHANGE_HISTORY);

	RepState->HistoryStart = RepState->HistoryStart % FRepState::MAX_CHANGE_HISTORY;
	RepState->HistoryEnd = RepState->HistoryStart + NewHistoryCount;
}
}

USpatialActorChannel::USpatialActorChannel(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
	, ActorEntityId(0)
	, ReserveEntityIdRequestId(-1)
	, CreateEntityRequestId(-1)
	, SpatialNetDriver(nullptr)
{
	bCoreActor = true;
	bCreatingNewEntity = false;
}

void USpatialActorChannel::Init(UNetConnection* Connection, int32 ChannelIndex, bool bOpenedLocally)
{
	Super::Init(Connection, ChannelIndex, bOpenedLocally);

	SpatialNetDriver = Cast<USpatialNetDriver>(Connection->Driver);
	check(SpatialNetDriver);

	WorkerView = SpatialNetDriver->GetSpatialOS()->GetView();
	WorkerConnection = SpatialNetDriver->GetSpatialOS()->GetConnection();

	BindToSpatialView();
}

void USpatialActorChannel::BindToSpatialView()
{
	if (SpatialNetDriver->ServerConnection)
	{
		// Don't need to bind to reserve/create entity responses on the client.
		return;
	}

	TSharedPtr<worker::View> PinnedView = WorkerView.Pin();
	if (PinnedView.IsValid())
	{
		ReserveEntityCallback = PinnedView->OnReserveEntityIdResponse([this](const worker::ReserveEntityIdResponseOp& Op)
		{
			if (Op.RequestId == ReserveEntityIdRequestId)
			{
				OnReserveEntityIdResponse(Op);
			}			
		});
		CreateEntityCallback = PinnedView->OnCreateEntityResponse([this](const worker::CreateEntityResponseOp& Op)
		{
			if (Op.RequestId == CreateEntityRequestId)
			{
				OnCreateEntityResponse(Op);
			}
		});
	}
}

void USpatialActorChannel::UnbindFromSpatialView() const
{
	//todo-giray: Uncomment the rest when worker sdk finishes the FR that gracefully handles removing unbound callback keys.
	return;
	/*
	TSharedPtr<worker::View> PinnedView = WorkerView.Pin();
	PinnedView->Remove(ReserveEntityCallback);
	PinnedView->Remove(CreateEntityCallback);*/
}

bool USpatialActorChannel::CleanUp(const bool bForDestroy)
{
	//todo-giray: This logic will not hold up when we have worker migration, needs to be revisited.
	if (Connection->Driver->IsServer())
	{
		TSharedPtr<worker::Connection> PinnedConnection = WorkerConnection.Pin();
		if (PinnedConnection.IsValid())
		{
			PinnedConnection->SendDeleteEntityRequest(ActorEntityId, 0);
		}
	}

	UnbindFromSpatialView();
	return UActorChannel::CleanUp(bForDestroy);
}

bool USpatialActorChannel::ReplicateActor()
{
	if (!IsReadyForReplication())
	{
		return false;
	}
	
	check(Actor);
	check(!Closing);
	check(Connection);
	check(Connection->PackageMap);
	
	const UWorld* const ActorWorld = Actor->GetWorld();

	USpatialInterop* Interop = SpatialNetDriver->GetSpatialInterop();
	check(Interop);

	// Time how long it takes to replicate this particular actor
	STAT(FScopeCycleCounterUObject FunctionScope(Actor));

	// Create an outgoing bunch (to satisfy some of the functions below).
	FOutBunch Bunch(this, 0);
	if (Bunch.IsError())
	{
		return false;
	}

	bIsReplicatingActor = true;
	FReplicationFlags RepFlags;

	// Send initial stuff.
	if (OpenPacketId.First == INDEX_NONE)
	{
		RepFlags.bNetInitial = true;
		Bunch.bClose = Actor->bNetTemporary;
		Bunch.bReliable = true; // Net temporary sends need to be reliable as well to force them to retry
	}

	//Here, Unreal would have determined if this connection belongs to this actor's Outer.
	//We don't have this concept when it comes to connections, our ownership-based logic is in the interop layer.
	//Setting this to true, but should not matter in the end.
	RepFlags.bNetOwner = true;

	// If initial, send init data.
	if (RepFlags.bNetInitial && OpenedLocally)
	{
		// TODO(David): Note that initial actor data is stored in the header to encode the NetGUID/Class/etc.
		// We don't care about this as we can distinguish this already based on the components in the entity,
		// so SerializeNewActor will probably do nothing.
		Connection->PackageMap->SerializeNewActor(Bunch, this, Actor);
	
		Actor->OnSerializeNewActor(Bunch);
	}

	RepFlags.bNetSimulated = (Actor->GetRemoteRole() == ROLE_SimulatedProxy);
	RepFlags.bRepPhysics = Actor->ReplicatedMovement.bRepPhysics;
	RepFlags.bReplay = ActorWorld && (ActorWorld->DemoNetDriver == Connection->GetDriver());
	RepFlags.bNetInitial = RepFlags.bNetInitial;

	UE_LOG(LogNetTraffic, Log, TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d"), *Actor->GetName(), RepFlags.bNetInitial, RepFlags.bNetOwner);

	FMemMark MemMark(FMemStack::Get());	// The calls to ReplicateProperties will allocate memory on FMemStack::Get(), and use it in ::PostSendBunch. we free it below

	// ----------------------------------------------------------
	// Replicate Actor and Component properties and RPCs
	// ----------------------------------------------------------

	// Epic does this at the net driver level, per connection. See UNetDriver::ServerReplicateActors().
	// However, we have many player controllers sharing one connection, so we do it at the actor level before replication.
	APlayerController* PC = Cast<APlayerController>(Actor);
	if (PC)
	{
		PC->SendClientAdjustment();
	}
	
	FRepChangelistState* ChangelistState = ActorReplicator->ChangelistMgr->GetRepChangelistState();
	bool bWroteSomethingImportant = false;

	ActorReplicator->ChangelistMgr->Update(Actor, Connection->Driver->ReplicationFrame, ActorReplicator->RepState->LastCompareIndex, RepFlags, bForceCompareProperties);

	const int32 PossibleNewHistoryIndex = ActorReplicator->RepState->HistoryEnd % FRepState::MAX_CHANGE_HISTORY;
	FRepChangedHistory& PossibleNewHistoryItem = ActorReplicator->RepState->ChangeHistory[PossibleNewHistoryIndex];
	TArray<uint16>& Changed = PossibleNewHistoryItem.Changed;

	// Gather all change lists that are new since we last looked, and merge them all together into a single CL
	for (int32 i = ActorReplicator->RepState->LastChangelistIndex; i < ChangelistState->HistoryEnd; i++)
	{
		const int32 HistoryIndex = i % FRepChangelistState::MAX_CHANGE_HISTORY;

		FRepChangedHistory& HistoryItem = ChangelistState->ChangeHistory[HistoryIndex];

		TArray<uint16> Temp = Changed;
		ActorReplicator->RepLayout->MergeChangeList((uint8*)Actor, HistoryItem.Changed, Temp, Changed);
	}

	const bool bCompareIndexSame = ActorReplicator->RepState->LastCompareIndex == ChangelistState->CompareIndex;
	ActorReplicator->RepState->LastCompareIndex = ChangelistState->CompareIndex;

	// We can early out if we know for sure there are no new changelists to send
	if (bCompareIndexSame || ActorReplicator->RepState->LastChangelistIndex == ChangelistState->HistoryEnd)
	{
		UpdateChangelistHistory(ActorReplicator->RepState);
		MemMark.Pop();
		return false;
	}

	//todo-giray: We currently don't take replication of custom delta properties into account here because it doesn't use changelists.
	// see ActorReplicator->ReplicateCustomDeltaProperties().

	if (RepFlags.bNetInitial || Changed.Num() > 0)
	{		
		if (RepFlags.bNetInitial && bCreatingNewEntity)
		{
			// When a player is connected, a FUniqueNetIdRepl is created with the players worker ID. This eventually gets stored
			// inside APlayerState::UniqueId when UWorld::SpawnPlayActor is called. If this actor channel is managing a pawn or a 
			// player controller, get the player state.
			FString PlayerWorkerId;
			APlayerState* PlayerState = Cast<APlayerState>(Actor);
			if (!PlayerState)
			{
				APawn* Pawn = Cast<APawn>(Actor);
				if (Pawn)
				{
					PlayerState = Pawn->PlayerState;
				}
			}
			if (!PlayerState)
			{
				APlayerController* PlayerController = Cast<APlayerController>(Actor);
				if (PlayerController)
				{
					PlayerState = PlayerController->PlayerState;
				}
			}
			if (PlayerState)
			{
				PlayerWorkerId = PlayerState->UniqueId.ToString();
			}
			else
			{
				UE_LOG(LogSpatialOSActorChannel, Log, TEXT("Unable to find PlayerState for %s, this usually means that this actor is not owned by a player."), *Actor->GetClass()->GetName());
			}

			// Ensure that the initial changelist contains _every_ property. This ensures that the default properties are written to the entity template.
			// Otherwise, there will be a mismatch between the rep state shadow data used by CompareProperties and the entity in SpatialOS.
			TArray<uint16> InitialChanged;
			for (auto& Cmd : ActorReplicator->RepLayout->Cmds)
			{
				if (Cmd.Type != REPCMD_DynamicArray && Cmd.Type != REPCMD_Return)
				{
					InitialChanged.Add(Cmd.RelativeHandle);
				}
			}
			InitialChanged.Add(0);

			// Calculate initial spatial position (but don't send component update) and create the entity.
			LastSpatialPosition = GetActorSpatialPosition(Actor);
			CreateEntityRequestId = Interop->SendCreateEntityRequest(this, LastSpatialPosition, PlayerWorkerId, InitialChanged);
			bCreatingNewEntity = false;
		}
		else
		{
			Interop->SendSpatialUpdate(this, Changed);
		}

		bWroteSomethingImportant = true;
		ActorReplicator->RepState->HistoryEnd++;
		UpdateChangelistHistory(ActorReplicator->RepState);
	}

	// Update SpatialOS position.
	if (!PC)
	{
		UpdateSpatialPosition();
	}

	ActorReplicator->RepState->LastChangelistIndex = ChangelistState->HistoryEnd;
	//todo-giray: The rest of this function is taken from Unreal's own implementation. It is mostly redundant in our case,
	// but keeping it here for now to give us a chance to investigate if we need to write our own implementation for any of
	// any code block below.

	/*
	// The Actor
	WroteSomethingImportant |= ActorReplicator->ReplicateProperties(Bunch, RepFlags);

	//todo-giray: Implement subobject replication
	// The SubObjects
	WroteSomethingImportant |= Actor->ReplicateSubobjects(this, &Bunch, &RepFlags);
*/

	// Look for deleted subobjects
	for (auto RepComp = ReplicationMap.CreateIterator(); RepComp; ++RepComp)
	{
		if (!RepComp.Key().IsValid())
		{
			// Write a deletion content header:
			WriteContentBlockForSubObjectDelete(Bunch, RepComp.Value()->ObjectNetGUID);

			bWroteSomethingImportant = true;
			Bunch.bReliable = true;

			RepComp.Value()->CleanUp();
			RepComp.RemoveCurrent();
		}
	}

	// -----------------------------
	// Send if necessary
	// -----------------------------
	bool SentBunch = false;
	if (bWroteSomethingImportant)
	{
		FPacketIdRange PacketRange = SendBunch(&Bunch, 1);

		for (auto RepComp = ReplicationMap.CreateIterator(); RepComp; ++RepComp)
		{
			RepComp.Value()->PostSendBunch(PacketRange, Bunch.bReliable);
		}
		SentBunch = true;
	}

	// If we evaluated everything, mark LastUpdateTime, even if nothing changed.
	LastUpdateTime = Connection->Driver->Time;

	MemMark.Pop();

	bIsReplicatingActor = false;

	bForceCompareProperties = false;		// Only do this once per frame when set

	return bWroteSomethingImportant;
}

void USpatialActorChannel::SetChannelActor(AActor* InActor)
{
	Super::SetChannelActor(InActor);

	if (!bCoreActor)
	{
		return;
	}

	check(SpatialNetDriver->GetEntityRegistry());
	ActorEntityId = SpatialNetDriver->GetEntityRegistry()->GetEntityIdFromActor(InActor).ToSpatialEntityId();

	// If the entity registry has no entry for this actor, this means we need to create it.
	if (ActorEntityId == 0)
	{
		USpatialNetConnection* SpatialConnection = Cast<USpatialNetConnection>(Connection);
		check(SpatialConnection);

		// Mark this channel as being responsible for creating this entity once we have an entity ID.
		bCreatingNewEntity = true;

		// Reserve an entity ID for this channel.
		TSharedPtr<worker::Connection> PinnedConnection = WorkerConnection.Pin();
		if (PinnedConnection.IsValid())
		{
			ReserveEntityIdRequestId = PinnedConnection->SendReserveEntityIdRequest(0);
		}
		UE_LOG(LogSpatialOSActorChannel, Log, TEXT("Opened channel for actor %s with no entity ID. Initiated reserve entity ID. Request id: %d"),
			*InActor->GetName(), ReserveEntityIdRequestId.Id);
	}
	else
	{
		UE_LOG(LogSpatialOSActorChannel, Log, TEXT("Opened channel for actor %s with existing entity ID %llu."), *InActor->GetName(), ActorEntityId);
	}
}

void USpatialActorChannel::PreReceiveSpatialUpdate()
{
	Actor->PreNetReceive();
}

void USpatialActorChannel::PostReceiveSpatialUpdate(const TArray<UProperty*>& RepNotifies)
{
	Actor->PostNetReceive();
	ActorReplicator->RepNotifies = RepNotifies;
	ActorReplicator->CallRepNotifies(false);
}

void USpatialActorChannel::OnReserveEntityIdResponse(const worker::ReserveEntityIdResponseOp& Op)
{
	if (Op.StatusCode != worker::StatusCode::kSuccess)
	{
		UE_LOG(LogSpatialOSActorChannel, Error, TEXT("Failed to reserve entity id. Reason: %s"), UTF8_TO_TCHAR(Op.Message.c_str()));
		//todo: From now on, this actor channel will be useless. We need better error handling, or a retry mechanism here.
		UnbindFromSpatialView();
		return;
	}
	UE_LOG(LogSpatialOSActorChannel, Log, TEXT("Received entity id (%d) for: %s. Request id: %d"), Op.EntityId.value_or(0), *Actor->GetName(), ReserveEntityIdRequestId.Id);

	auto PinnedView = WorkerView.Pin();
	if (PinnedView.IsValid())
	{
		PinnedView->Remove(ReserveEntityCallback);
	}

	USpatialPackageMapClient* PackageMap = Cast<USpatialPackageMapClient>(Connection->PackageMap);
	check(PackageMap);
	ActorEntityId = *Op.EntityId;

	SpatialNetDriver->GetEntityRegistry()->AddToRegistry(ActorEntityId, GetActor());
	PackageMap->ResolveEntityActor(Actor, ActorEntityId);
}

void USpatialActorChannel::OnCreateEntityResponse(const worker::CreateEntityResponseOp& Op)
{
	check(SpatialNetDriver->GetNetMode() < NM_Client);

	if (Op.StatusCode != worker::StatusCode::kSuccess)
	{
		UE_LOG(LogSpatialOSActorChannel, Error, TEXT("Failed to create entity for actor %s: %s"), *Actor->GetName(), UTF8_TO_TCHAR(Op.Message.c_str()));
		//todo: From now on, this actor channel will be useless. We need better error handling, or a retry mechanism here.
		UnbindFromSpatialView();
		return;
	}
	UE_LOG(LogSpatialOSActorChannel, Log, TEXT("Created entity (%d) for: %s. Request id: %d"), ActorEntityId, *Actor->GetName(), ReserveEntityIdRequestId.Id);

	auto PinnedView = WorkerView.Pin();
	if (PinnedView.IsValid())
	{
		PinnedView->Remove(CreateEntityCallback);
	}

	UE_LOG(LogSpatialOSActorChannel, Log, TEXT("Received create entity response op for %d"), ActorEntityId);	
}	

void USpatialActorChannel::UpdateSpatialPosition()
{
	// PlayerController's are a special case here. To ensure that the PlayerController and its pawn is migrated
	// between workers at the same time (which is not guaranteed), we ensure that we update the position component of
	// the PlayerController at the same time as the pawn.

	// Check that it has moved sufficiently far to be updated
	const float SpatialPositionThreshold = 100.0f * 100.0f; // 1m (100cm)
	FVector ActorSpatialPosition = GetActorSpatialPosition(Actor);
	if (FVector::DistSquared(ActorSpatialPosition, LastSpatialPosition) < SpatialPositionThreshold)
	{
		return;
	}

	USpatialInterop* Interop = SpatialNetDriver->GetSpatialInterop();

	LastSpatialPosition = ActorSpatialPosition;
	Interop->SendSpatialPositionUpdate(GetEntityId(), LastSpatialPosition);

	// If we're a pawn and are controlled by a player controller, update the player controller and the player state positions too.
	APawn* Pawn = Cast<APawn>(Actor);
	if (Pawn && Cast<APlayerController>(Pawn->GetController()))
	{
			AController* Controller = Pawn->GetController();
			if (Pawn->GetController()) 
			{
					USpatialActorChannel* ControllerActorChannel = Cast<USpatialActorChannel>(Connection->ActorChannels.FindRef(Pawn->GetController()));
					if (ControllerActorChannel)
					{
							Interop->SendSpatialPositionUpdate(ControllerActorChannel->GetEntityId(), LastSpatialPosition);
					}
					USpatialActorChannel* PlayerStateActorChannel = Cast<USpatialActorChannel>(Connection->ActorChannels.FindRef(Pawn->GetController()->PlayerState));
					if (PlayerStateActorChannel)
					{
							Interop->SendSpatialPositionUpdate(PlayerStateActorChannel->GetEntityId(), LastSpatialPosition);
					}
			}
	}
}

FVector USpatialActorChannel::GetActorSpatialPosition(AActor* Actor)
{
	// Preferentially uses the owner location over the origin
	// This is to enable actors like PlayerState to follow their corresponding character

	// If the actor has a well defined location then use that
	// Otherwise if it has a parent use its location
	// Otherwise use the origin
	if (Actor->GetRootComponent()) 
	{
		return Actor->GetRootComponent()->GetComponentLocation();
	}
	else if (Actor->GetOwner())
	{
		return GetActorSpatialPosition(Actor->GetOwner());
	}
	else
	{
		return FVector::ZeroVector;
	}
}
