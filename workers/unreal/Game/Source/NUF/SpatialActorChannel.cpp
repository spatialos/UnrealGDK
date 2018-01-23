// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialActorChannel.h"
#include "SpatialNetDriver.h"
#include "SpatialPackageMapClient.h"
#include "EntityRegistry.h"
#include "Engine/DemoNetDriver.h"
#include "Net/DataBunch.h"
#include "Net/NetworkProfiler.h"
#include <improbable/worker.h>
#include <improbable/standard_library.h>
#include <improbable/player/player.h>
#include "Commander.h"
#include "EntityBuilder.h"
#include "EntityTemplate.h"
#include "SpatialNetConnection.h"
#include "SpatialOS.h"
#include "SpatialUpdateInterop.h"
#include "SpatialNetDriver.h"

using namespace improbable;

DEFINE_LOG_CATEGORY(LogSpatialOSActorChannel);

USpatialActorChannel::USpatialActorChannel(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
{
	ChType = CHTYPE_Actor;
	bCoreActor = true;
	bSendingInitialBunch = false;
	ActorEntityId = worker::EntityId{};
}

void USpatialActorChannel::SendCreateEntityRequest(const TArray<uint16>& Changed)
{
	TSharedPtr<worker::Connection> PinnedConnection = WorkerConnection.Pin();
	if (PinnedConnection.IsValid())
	{
		USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);
		checkf(Driver->GetSpatialUpdateInterop(), TEXT("Spatial Update Interop is not initialised"));

		const USpatialTypeBinding* TypeBinding = Driver->GetSpatialUpdateInterop()->GetTypeBindingByClass(Actor->GetClass());

		FStringAssetReference ActorClassRef(Actor->GetClass());
		FString PathStr = ActorClassRef.ToString();

		if (TypeBinding)
		{
			auto Entity = TypeBinding->CreateActorEntity(Actor->GetActorLocation(), PathStr, GetChangeState(Changed));
			CreateEntityRequestId = PinnedConnection->SendCreateEntityRequest(Entity, ActorEntityId, 0);
		}
		else
		{
			WorkerAttributeSet UnrealWorkerAttributeSet{ { worker::List<std::string>{"UnrealWorker"} } };
			WorkerAttributeSet UnrealClientAttributeSet{ { worker::List<std::string>{"UnrealClient"} } };

			// UnrealWorker write authority, any worker read authority
			WorkerRequirementSet UnrealWorkerWritePermission{ { UnrealWorkerAttributeSet } };
			WorkerRequirementSet UnrealClientWritePermission{ { UnrealClientAttributeSet } };
			WorkerRequirementSet AnyWorkerReadRequirement{ { UnrealWorkerAttributeSet, UnrealClientAttributeSet } };

			auto Entity = unreal::FEntityBuilder::Begin()
				.AddPositionComponent(USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(Actor->GetActorLocation()), UnrealWorkerWritePermission)
				.AddMetadataComponent(Metadata::Data{ TCHAR_TO_UTF8(*PathStr) })
				.SetPersistence(true)
				.SetReadAcl(AnyWorkerReadRequirement)
				// For now, just a dummy component we add to every such entity to make sure client has write access to at least one component.
				//todo-giray: Remove once we're using proper (generated) entity templates here.
				.AddComponent<improbable::player::PlayerControlClient>(improbable::player::PlayerControlClientData{}, UnrealClientWritePermission)
				.Build();

			CreateEntityRequestId = PinnedConnection->SendCreateEntityRequest(Entity, ActorEntityId, 0);
		}
		UE_LOG(LogSpatialOSActorChannel, Log, TEXT("Creating entity for actor %s. Request id: %d. Entity id: %d"), *Actor->GetName(), CreateEntityRequestId.Id, ActorEntityId);
	}
	else
	{
		UE_LOG(LogSpatialOSActorChannel, Warning, TEXT("Failed to obtain reference to SpatialOS connection!"));
		return;
	}
}

void USpatialActorChannel::Init(UNetConnection* Connection, int32 ChannelIndex, bool bOpenedLocally)
{
	Super::Init(Connection, ChannelIndex, bOpenedLocally);

	USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);
	check(Driver);

	WorkerView = Driver->GetSpatialOS()->GetView();
	WorkerConnection = Driver->GetSpatialOS()->GetConnection();

	BindToSpatialView();
}

void USpatialActorChannel::BindToSpatialView()
{
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
	USpatialNetConnection* SpatialConnection = Cast<USpatialNetConnection>(Connection);
	if (SpatialConnection && SpatialConnection->Driver->IsServer())
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

//This is a bookkeeping function that is similar to the one in RepLayout.cpp, modified for our needs (e.g. no NaKs)
// We can't use the one in RepLayout.cpp because it's private and it cannot account for our approach.
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

	// The package map shouldn't have any carry over guids
	if (CastChecked<UPackageMapClient>(Connection->PackageMap)->GetMustBeMappedGuidsInLastBunch().Num() != 0)
	{
		UE_LOG(LogNet, Warning, TEXT("ReplicateActor: PackageMap->GetMustBeMappedGuidsInLastBunch().Num() != 0: %i"), CastChecked<UPackageMapClient>(Connection->PackageMap)->GetMustBeMappedGuidsInLastBunch().Num());
	}

	// Time how long it takes to replicate this particular actor
	STAT(FScopeCycleCounterUObject FunctionScope(Actor));

	// Create an outgoing bunch, and skip this actor if the channel is saturated.
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

	FMemMark	MemMark(FMemStack::Get());	// The calls to ReplicateProperties will allocate memory on FMemStack::Get(), and use it in ::PostSendBunch. we free it below

											// ----------------------------------------------------------
											// Replicate Actor and Component properties and RPCs
											// ----------------------------------------------------------

	bool WroteSomethingImportant = false;

	ActorReplicator->ChangelistMgr->Update(Actor, Connection->Driver->ReplicationFrame, ActorReplicator->RepState->LastCompareIndex, RepFlags, bForceCompareProperties);

	const int32 PossibleNewHistoryIndex = ActorReplicator->RepState->HistoryEnd % FRepState::MAX_CHANGE_HISTORY;

	FRepChangedHistory& PossibleNewHistoryItem = ActorReplicator->RepState->ChangeHistory[PossibleNewHistoryIndex];

	TArray<uint16>& Changed = PossibleNewHistoryItem.Changed;

	FRepChangelistState* ChangelistState = ActorReplicator->ChangelistMgr->GetRepChangelistState();
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
		return false;
	}

	//todo-giray: We currently don't take replication of custom delta properties into account here because it doesn't use changelists.
	// see ActorReplicator->ReplicateCustomDeltaProperties().

	if (Changed.Num() > 0)
	{
		USpatialUpdateInterop* UpdateInterop = Cast<USpatialNetDriver>(Connection->Driver)->GetSpatialUpdateInterop();
		check(UpdateInterop);
		if (RepFlags.bNetInitial)
		{
			SendCreateEntityRequest(Changed);
		}
		else
		{
			UpdateInterop->SendSpatialUpdate(this, Changed);
		}
		WroteSomethingImportant = true;
		ActorReplicator->RepState->HistoryEnd++;
		UpdateChangelistHistory(ActorReplicator->RepState);

		// TODO(David): We want to create the entity or send a SpatialOS update here.
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

			WroteSomethingImportant = true;
			Bunch.bReliable = true;

			RepComp.Value()->CleanUp();
			RepComp.RemoveCurrent();
		}
	}

	// -----------------------------
	// Send if necessary
	// -----------------------------
	bool SentBunch = false;
	if (WroteSomethingImportant)
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

	return WroteSomethingImportant;
}

void USpatialActorChannel::SetChannelActor(AActor* InActor)
{
	Super::SetChannelActor(InActor);

	if (!bCoreActor)
		return;

	USpatialNetConnection* SpatialConnection = Cast<USpatialNetConnection>(Connection);

	if (SpatialConnection && SpatialConnection->Driver->IsServer())
	{
		// Reserve an entity ID for this channel.
		TSharedPtr<worker::Connection> PinnedConnection = WorkerConnection.Pin();
		if (PinnedConnection.IsValid())
		{
			ReserveEntityIdRequestId = PinnedConnection->SendReserveEntityIdRequest(0);
		}
		UE_LOG(LogSpatialOSActorChannel, Log, TEXT("Initiated reserve entity process for: %s. Request id: %d"), *InActor->GetName(), ReserveEntityIdRequestId.Id);
	}
	else
	{
		USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);
		check(Driver);
		check(Driver->GetEntityRegistry());
		ActorEntityId = Driver->GetEntityRegistry()->GetEntityIdFromActor(InActor).ToSpatialEntityId();
		UE_LOG(LogSpatialOSActorChannel, Log, TEXT("Opened non-authoritative channel for actor %s."), *InActor->GetName());
	}
}

void USpatialActorChannel::OnReserveEntityIdResponse(const worker::ReserveEntityIdResponseOp& Op)
{
	if (Op.StatusCode != worker::StatusCode::kSuccess)
	{
		UE_LOG(LogSpatialOSActorChannel, Error, TEXT("Failed to reserve entity id: %s"), UTF8_TO_TCHAR(Op.Message.c_str()));
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

	ActorEntityId = *Op.EntityId;

	USpatialPackageMapClient* SpatialPMC = Cast<USpatialPackageMapClient>(Connection->PackageMap);
	check(SpatialPMC);
	SpatialPMC->ResolveEntityActor(Actor, ActorEntityId);
}

void USpatialActorChannel::OnCreateEntityResponse(const worker::CreateEntityResponseOp& Op)
{
	if (Op.StatusCode != worker::StatusCode::kSuccess)
	{
		UE_LOG(LogSpatialOSActorChannel, Error, TEXT("Failed to create entity for actor %s: %s"), *Actor->GetName(), UTF8_TO_TCHAR(Op.Message.c_str()));
		//todo: From now on, this actor channel will be useless. We need better error handling, or a retry mechanism here.
		UnbindFromSpatialView();
		return;
	}
	UE_LOG(LogSpatialOSActorChannel, Log, TEXT("Created entity (%d) for: %s. Request id: %d"), Op.EntityId.value_or(0), *Actor->GetName(), ReserveEntityIdRequestId.Id);

	USpatialNetDriver* Driver = Cast<USpatialNetDriver>(Connection->Driver);
	USpatialNetConnection* SpatialConnection = Driver->GetSpatialOSNetConnection();

	auto PinnedView = WorkerView.Pin();
	if (PinnedView.IsValid())
	{
		PinnedView->Remove(CreateEntityCallback);
	}

	// This can be true only on the server
	if (SpatialConnection)
	{
		USpatialPackageMapClient* PMC = Cast<USpatialPackageMapClient>(SpatialConnection->PackageMap);
		if (PMC)
		{
			worker::EntityId SpatialEntityId = Op.EntityId.value_or(0);
			FEntityId EntityId(SpatialEntityId);

			UE_LOG(LogSpatialOSActorChannel, Log, TEXT("Received create entity response op for %d"), EntityId.ToSpatialEntityId());
			// once we know the entity was successfully spawned, add the local actor 
			// to the package map and to the EntityRegistry
			PMC->ResolveEntityActor(GetActor(), EntityId);
			Driver->GetEntityRegistry()->AddToRegistry(EntityId, GetActor());
			ActorEntityId = SpatialEntityId;
		}
	}
}	
