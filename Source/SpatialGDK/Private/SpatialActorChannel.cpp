// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialActorChannel.h"
#include "Engine/DemoNetDriver.h"
#include "EntityRegistry.h"
#include "GameFramework/PlayerState.h"
#include "Net/DataBunch.h"
#include "Net/NetworkProfiler.h"
#include "SpatialConstants.h"
#include "SpatialInterop.h"
#include "SpatialNetConnection.h"
#include "SpatialNetDriver.h"
#include "SpatialOS.h"
#include "SpatialPackageMapClient.h"
#include "SpatialTypeBinding.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKActorChannel);

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

void USpatialActorChannel::Init(UNetConnection* InConnection, int32 ChannelIndex, bool bOpenedLocally)
{
	Super::Init(InConnection, ChannelIndex, bOpenedLocally);

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

void USpatialActorChannel::DeleteEntityIfAuthoritative()
{
	bool bHasAuthority = false;

	TSharedPtr<worker::View> PinnedView = WorkerView.Pin();
	if (PinnedView.IsValid())
	{
		bHasAuthority = PinnedView->GetAuthority<improbable::Position>(ActorEntityId.ToSpatialEntityId()) == worker::Authority::kAuthoritative;
	}

	// If we have authority and aren't trying to delete the spawner, delete the entity
	if (bHasAuthority && ActorEntityId.ToSpatialEntityId() != SpatialConstants::EntityIds::SPAWNER_ENTITY_ID)
	{
		USpatialInterop* Interop = SpatialNetDriver->GetSpatialInterop();
		Interop->DeleteEntity(ActorEntityId);
	}
}

bool USpatialActorChannel::CleanUp(const bool bForDestroy)
{
	UnbindFromSpatialView();

#if WITH_EDITOR
	if (SpatialNetDriver->IsServer() &&
		SpatialNetDriver->GetWorld()->WorldType == EWorldType::PIE &&
		SpatialNetDriver->GetEntityRegistry()->GetActorFromEntityId(ActorEntityId.ToSpatialEntityId()))
	{
		// If we're running in PIE, as a server worker, and the entity hasn't already been cleaned up, delete it on shutdown.
		DeleteEntityIfAuthoritative();
	}
#endif

	return UActorChannel::CleanUp(bForDestroy);
}

void USpatialActorChannel::Close()
{
	DeleteEntityIfAuthoritative();
	Super::Close();
}

FPropertyChangeState USpatialActorChannel::CreateSubobjectChangeState(UActorComponent* Component)
{
	FObjectReplicator& Replicator = FindOrCreateReplicator(TWeakObjectPtr<UObject>(Component)).Get();

	TArray<uint16> InitialRepChanged;
	int32 DynamicArrayDepth = 0;
	const int32 CmdCount = Replicator.RepLayout->Cmds.Num();
	for (uint16 CmdIdx = 0; CmdIdx < CmdCount; ++CmdIdx)
	{
		const auto& Cmd = Replicator.RepLayout->Cmds[CmdIdx];

		InitialRepChanged.Add(Cmd.RelativeHandle);

		if (Cmd.Type == REPCMD_DynamicArray)
		{
			DynamicArrayDepth++;

			// For the first layer of each dynamic array encountered at the root level
			// add the number of array properties to conform to Unreal's RepLayout design and 
			// allow FRepHandleIterator to jump over arrays. Cmd.EndCmd is an index into 
			// RepLayout->Cmds[] that points to the value after the termination NULL of this array.
			if (DynamicArrayDepth == 1)
			{
				InitialRepChanged.Add((Cmd.EndCmd - CmdIdx) - 2);
			}
		}
		else if (Cmd.Type == REPCMD_Return)
		{
			DynamicArrayDepth--;
			checkf(DynamicArrayDepth >= 0 || CmdIdx == CmdCount - 1, TEXT("Encountered erroneous RepLayout"));
		}
	}

	return GetChangeStateSubobject(Component, &Replicator, InitialRepChanged, TArray<uint16>());
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
	APlayerController* PlayerController = Cast<APlayerController>(Actor);
	if (PlayerController)
	{
		PlayerController->SendClientAdjustment();
	}

	// Update SpatialOS position.
	if (!PlayerController && !Cast<APlayerState>(Actor))
	{
		UpdateSpatialPosition();
	}
	
	// Update the replicated property change list.
	FRepChangelistState* ChangelistState = ActorReplicator->ChangelistMgr->GetRepChangelistState();
	bool bWroteSomethingImportant = false;
	ActorReplicator->ChangelistMgr->Update(Actor, Connection->Driver->ReplicationFrame, ActorReplicator->RepState->LastCompareIndex, RepFlags, bForceCompareProperties);

	const int32 PossibleNewHistoryIndex = ActorReplicator->RepState->HistoryEnd % FRepState::MAX_CHANGE_HISTORY;
	FRepChangedHistory& PossibleNewHistoryItem = ActorReplicator->RepState->ChangeHistory[PossibleNewHistoryIndex];
	TArray<uint16>& RepChanged = PossibleNewHistoryItem.Changed;

	// Gather all change lists that are new since we last looked, and merge them all together into a single CL
	for (int32 i = ActorReplicator->RepState->LastChangelistIndex; i < ChangelistState->HistoryEnd; i++)
	{
		const int32 HistoryIndex = i % FRepChangelistState::MAX_CHANGE_HISTORY;
		FRepChangedHistory& HistoryItem = ChangelistState->ChangeHistory[HistoryIndex];
		TArray<uint16> Temp = RepChanged;
		ActorReplicator->RepLayout->MergeChangeList((uint8*)Actor, HistoryItem.Changed, Temp, RepChanged);
	}

	const bool bCompareIndexSame = ActorReplicator->RepState->LastCompareIndex == ChangelistState->CompareIndex;
	ActorReplicator->RepState->LastCompareIndex = ChangelistState->CompareIndex;

	// Update the migratable property change list.
	USpatialTypeBinding* Binding = Interop->GetTypeBindingByClass(Actor->GetClass());
	TArray<uint16> MigratableChanged;
	if (Binding)
	{
		uint32 ShadowDataOffset = 0;
		for (auto& PropertyInfo : Binding->GetMigratableHandlePropertyMap())
		{
			const uint8* Data = PropertyInfo.Value.GetPropertyData((uint8*)Actor);

			// Compare and assign.
			if (RepFlags.bNetInitial || !PropertyInfo.Value.Property->Identical(MigratablePropertyShadowData.GetData() + ShadowDataOffset, Data))
			{
				MigratableChanged.Add(PropertyInfo.Key);
				PropertyInfo.Value.Property->CopyCompleteValue(MigratablePropertyShadowData.GetData() + ShadowDataOffset, Data);
			}
			ShadowDataOffset += PropertyInfo.Value.Property->GetSize();
		}
	}

	// We can early out if we know for sure there are no new changelists to send, and we are not creating a new entity.
	bool bReplicateActor = true;
	if (!bCreatingNewEntity && MigratableChanged.Num() == 0)
	{
		if (bCompareIndexSame || ActorReplicator->RepState->LastChangelistIndex == ChangelistState->HistoryEnd)
		{
			UpdateChangelistHistory(ActorReplicator->RepState);
			MemMark.Pop();
			bReplicateActor = false;
		}
	}

	//todo-giray: We currently don't take replication of custom delta properties into account here because it doesn't use changelists.
	// see ActorReplicator->ReplicateCustomDeltaProperties().

	// If any properties have changed, send a component update.
	if (bReplicateActor && RepFlags.bNetInitial || RepChanged.Num() > 0 || MigratableChanged.Num() > 0)
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
				UE_LOG(LogSpatialGDKActorChannel, Log, TEXT("Unable to find PlayerState for %s, this usually means that this actor is not owned by a player."), *Actor->GetClass()->GetName());
			}

			// Ensure that the initial changelist contains _every_ property. This ensures that the default properties are written to the entity template.
			// Otherwise, there will be a mismatch between the rep state shadow data used by CompareProperties and the entity in SpatialOS.
			TArray<uint16> InitialRepChanged;
			int32 DynamicArrayDepth = 0;
			const int32 CmdCount = ActorReplicator->RepLayout->Cmds.Num();
			for (uint16 CmdIdx = 0; CmdIdx < CmdCount; ++CmdIdx)
			{
				const auto& Cmd = ActorReplicator->RepLayout->Cmds[CmdIdx];

				InitialRepChanged.Add(Cmd.RelativeHandle);

				if (Cmd.Type == REPCMD_DynamicArray)
				{
					DynamicArrayDepth++;

					// For the first layer of each dynamic array encountered at the root level
					// add the number of array properties to conform to Unreal's RepLayout design and 
					// allow FRepHandleIterator to jump over arrays. Cmd.EndCmd is an index into 
					// RepLayout->Cmds[] that points to the value after the termination NULL of this array.
					if (DynamicArrayDepth == 1)
					{
						InitialRepChanged.Add((Cmd.EndCmd - CmdIdx) - 2);
					}
				}
				else if (Cmd.Type == REPCMD_Return)
				{
					DynamicArrayDepth--;
					checkf(DynamicArrayDepth >= 0 || CmdIdx == CmdCount - 1, TEXT("Encountered erroneous RepLayout"));
				}
			}

			// Calculate initial spatial position (but don't send component update) and create the entity.
			LastSpatialPosition = GetActorSpatialPosition(Actor);
			CreateEntityRequestId = Interop->SendCreateEntityRequest(this, LastSpatialPosition, PlayerWorkerId, InitialRepChanged, MigratableChanged);
			bCreatingNewEntity = false;
		}
		else
		{
			Interop->SendSpatialUpdate(this, RepChanged, MigratableChanged);
		}

		bWroteSomethingImportant = true;
		if (RepChanged.Num() > 0)
		{
			ActorReplicator->RepState->HistoryEnd++;
		}
		UpdateChangelistHistory(ActorReplicator->RepState);
	}

	ActorReplicator->RepState->LastChangelistIndex = ChangelistState->HistoryEnd;

	for (UActorComponent* ActorComp : Actor->GetReplicatedComponents())
	{
		USpatialTypeBinding* ComponentTypeBinding = Interop->GetTypeBindingByClass(ActorComp->GetClass());
		if (ActorComp && ActorComp->GetIsReplicated() && ComponentTypeBinding) // Only replicated subobjects with type bindings
		{
			bWroteSomethingImportant |= ReplicateSubobject(ActorComp, RepFlags);
		}
	}

	/*
	// Do we need to add support for deleted subobjects?
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
	}*/

	// If we evaluated everything, mark LastUpdateTime, even if nothing changed.
	LastUpdateTime = Connection->Driver->Time;

	MemMark.Pop();

	bIsReplicatingActor = false;

	bForceCompareProperties = false;		// Only do this once per frame when set

	return bWroteSomethingImportant;
}

bool USpatialActorChannel::ReplicateSubobject(UObject *Obj, const FReplicationFlags &RepFlags)
{

	FObjectReplicator& replicator = FindOrCreateReplicator(TWeakObjectPtr<UObject>(Obj)).Get();
	FRepChangelistState* ChangelistState = replicator.ChangelistMgr->GetRepChangelistState();
	replicator.ChangelistMgr->Update(Obj, replicator.Connection->Driver->ReplicationFrame, replicator.RepState->LastCompareIndex, RepFlags, bForceCompareProperties);

	const int32 PossibleNewHistoryIndex = replicator.RepState->HistoryEnd % FRepState::MAX_CHANGE_HISTORY;
	FRepChangedHistory& PossibleNewHistoryItem = replicator.RepState->ChangeHistory[PossibleNewHistoryIndex];
	TArray<uint16>& RepChanged = PossibleNewHistoryItem.Changed;

	// Gather all change lists that are new since we last looked, and merge them all together into a single CL
	for (int32 i = replicator.RepState->LastChangelistIndex; i < ChangelistState->HistoryEnd; i++)
	{
		const int32 HistoryIndex = i % FRepChangelistState::MAX_CHANGE_HISTORY;
		FRepChangedHistory& HistoryItem = ChangelistState->ChangeHistory[HistoryIndex];
		TArray<uint16> Temp = RepChanged;
		replicator.RepLayout->MergeChangeList((uint8*)Actor, HistoryItem.Changed, Temp, RepChanged);
	}

	const bool bCompareIndexSame = replicator.RepState->LastCompareIndex == ChangelistState->CompareIndex;
	replicator.RepState->LastCompareIndex = ChangelistState->CompareIndex;

	if (RepChanged.Num() > 0)
	{
		USpatialInterop* Interop = SpatialNetDriver->GetSpatialInterop();
		check(Interop);
		Interop->SendSpatialUpdateSubobject(this, Obj, &replicator, RepChanged, TArray<uint16>());
		replicator.RepState->HistoryEnd++;
	}

	UpdateChangelistHistory(replicator.RepState);
	replicator.RepState->LastChangelistIndex = ChangelistState->HistoryEnd;

	return true;
}

void USpatialActorChannel::SetChannelActor(AActor* InActor)
{
	Super::SetChannelActor(InActor);

	if (!bCoreActor)
	{
		return;
	}

	// Set up the shadow data for the migratable properties. This is used later to compare the properties and send only changed ones.
	USpatialInterop* Interop = SpatialNetDriver->GetSpatialInterop();
	check(Interop);
	USpatialTypeBinding* Binding = Interop->GetTypeBindingByClass(InActor->GetClass());
	if (Binding)
	{
		const FMigratableHandlePropertyMap& MigratableProperties = Binding->GetMigratableHandlePropertyMap();
		uint32 Size = 0;
		for (auto& Property : MigratableProperties)
		{
			Size += Property.Value.Property->GetSize();
		}
		MigratablePropertyShadowData.Empty();
		MigratablePropertyShadowData.AddZeroed(Size);
		uint32 Offset = 0;
		for (auto& Property : MigratableProperties)
		{
			Property.Value.Property->InitializeValue(MigratablePropertyShadowData.GetData() + Offset);
			Offset += Property.Value.Property->GetSize();
		}
	}

	// Get the entity ID from the entity registry (or return 0 if it doesn't exist).
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
		UE_LOG(LogSpatialGDKActorChannel, Log, TEXT("Opened channel for actor %s with no entity ID. Initiated reserve entity ID. Request id: %d"),
			*InActor->GetName(), ReserveEntityIdRequestId.Id);
	}
	else
	{
		UE_LOG(LogSpatialGDKActorChannel, Log, TEXT("Opened channel for actor %s with existing entity ID %lld."), *InActor->GetName(), ActorEntityId.ToSpatialEntityId());

		// Inform USpatialInterop of this new actor channel/entity pairing
		SpatialNetDriver->GetSpatialInterop()->AddActorChannel(ActorEntityId.ToSpatialEntityId(), this);
	}
}

void USpatialActorChannel::PreReceiveSpatialUpdate()
{
	Actor->PreNetReceive();
}

void USpatialActorChannel::PreReceiveSpatialUpdateSubobject(UActorComponent* Component)
{
	Component->PreNetReceive();
}

void USpatialActorChannel::PostReceiveSpatialUpdate(const TArray<UProperty*>& RepNotifies)
{
	Actor->PostNetReceive();
	ActorReplicator->RepNotifies = RepNotifies;
	ActorReplicator->CallRepNotifies(false);
}

void USpatialActorChannel::PostReceiveSpatialUpdateSubobject(UActorComponent* Component, const TArray<UProperty*>& RepNotifies)
{
	TWeakObjectPtr<UObject> Obj(Component);
	if (ReplicationMap.Find(Obj))
	{
		FObjectReplicator& Replicator = FindOrCreateReplicator(Obj).Get();
		Component->PostNetReceive();
		Replicator.RepNotifies = RepNotifies;
		Replicator.CallRepNotifies(false);
	}
}

void USpatialActorChannel::OnReserveEntityIdResponse(const worker::ReserveEntityIdResponseOp& Op)
{
	if (Op.StatusCode != worker::StatusCode::kSuccess)
	{
		UE_LOG(LogSpatialGDKActorChannel, Error, TEXT("Failed to reserve entity id. Reason: %s"), UTF8_TO_TCHAR(Op.Message.c_str()));
		//todo: From now on, this actor channel will be useless. We need better error handling, or a retry mechanism here.
		UnbindFromSpatialView();
		return;
	}
	UE_LOG(LogSpatialGDKActorChannel, Log, TEXT("Received entity id (%d) for: %s. Request id: %d"), Op.EntityId.value_or(0), *Actor->GetName(), ReserveEntityIdRequestId.Id);

	auto PinnedView = WorkerView.Pin();
	if (PinnedView.IsValid())
	{
		PinnedView->Remove(ReserveEntityCallback);
	}

	ActorEntityId = *Op.EntityId;

	SpatialNetDriver->GetEntityRegistry()->AddToRegistry(ActorEntityId, GetActor());

	// Inform USpatialInterop of this new actor channel/entity pairing
	SpatialNetDriver->GetSpatialInterop()->AddActorChannel(ActorEntityId.ToSpatialEntityId(), this);
}

void USpatialActorChannel::OnCreateEntityResponse(const worker::CreateEntityResponseOp& Op)
{
	check(SpatialNetDriver->GetNetMode() < NM_Client);

	if (Op.StatusCode != worker::StatusCode::kSuccess)
	{
		UE_LOG(LogSpatialGDKActorChannel, Error, TEXT("Failed to create entity for actor %s: %s"), *Actor->GetName(), UTF8_TO_TCHAR(Op.Message.c_str()));
		//todo: From now on, this actor channel will be useless. We need better error handling, or a retry mechanism here.
		UnbindFromSpatialView();
		return;
	}
	UE_LOG(LogSpatialGDKActorChannel, Log, TEXT("Created entity (%lld) for: %s. Request id: %d"), ActorEntityId.ToSpatialEntityId(), *Actor->GetName(), ReserveEntityIdRequestId.Id);

	auto PinnedView = WorkerView.Pin();
	if (PinnedView.IsValid())
	{
		PinnedView->Remove(CreateEntityCallback);
	}

	UE_LOG(LogSpatialGDKActorChannel, Log, TEXT("Received create entity response op for %lld"), ActorEntityId.ToSpatialEntityId());
}

void USpatialActorChannel::UpdateSpatialPosition()
{
	// PlayerController's and PlayerState's are a special case here. To ensure that they and their associated pawn are 
	// migrated between workers at the same time (which is not guaranteed), we ensure that we update the position component 
	// of the PlayerController and PlayerState at the same time as the pawn.

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
	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
		{
			USpatialActorChannel* ControllerActorChannel = Cast<USpatialActorChannel>(Connection->ActorChannels.FindRef(PlayerController));
			if (ControllerActorChannel)
			{
				Interop->SendSpatialPositionUpdate(ControllerActorChannel->GetEntityId(), LastSpatialPosition);
			}
			USpatialActorChannel* PlayerStateActorChannel = Cast<USpatialActorChannel>(Connection->ActorChannels.FindRef(PlayerController->PlayerState));
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
