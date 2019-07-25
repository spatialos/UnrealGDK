// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialActorChannel.h"

#include "Engine/DemoNetDriver.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/DataBunch.h"
#include "Net/NetworkProfiler.h"

#if WITH_EDITOR
#include "Settings/LevelEditorPlaySettings.h"
#endif

#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialSender.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/GlobalStateManager.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialActorUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialActorChannel);

DECLARE_CYCLE_STAT(TEXT("ReplicateActor"), STAT_SpatialActorChannelReplicateActor, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("UpdateSpatialPosition"), STAT_SpatialActorChannelUpdateSpatialPosition, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("ReplicateSubobject"), STAT_SpatialActorChannelReplicateSubobject, STATGROUP_SpatialNet);

namespace
{
// This is a bookkeeping function that is similar to the one in RepLayout.cpp, modified for our needs (e.g. no NaKs)
// We can't use the one in RepLayout.cpp because it's private and it cannot account for our approach.
// In this function, we poll for any changes in Unreal properties compared to the last time we replicated this actor.
#if ENGINE_MINOR_VERSION <= 20
void UpdateChangelistHistory(TSharedPtr<FRepState>& RepState)
#else
void UpdateChangelistHistory(TUniquePtr<FRepState>& RepState)
#endif
{
	check(RepState->HistoryEnd >= RepState->HistoryStart);

	const int32 HistoryCount = RepState->HistoryEnd - RepState->HistoryStart;
	check(HistoryCount < FRepState::MAX_CHANGE_HISTORY);

	for (int32 i = RepState->HistoryStart; i < RepState->HistoryEnd; i++)
	{
		const int32 HistoryIndex = i % FRepState::MAX_CHANGE_HISTORY;

		FRepChangedHistory & HistoryItem = RepState->ChangeHistory[HistoryIndex];

		// All active history items should contain a change list
		check(HistoryItem.Changed.Num() > 0);

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
	, bCreatedEntity(false)
	, bCreatingNewEntity(false)
	, EntityId(SpatialConstants::INVALID_ENTITY_ID)
	, bInterestDirty(false)
	, bIsListening(false)
	, bNetOwned(false)
	, NetDriver(nullptr)
	, LastPositionSinceUpdate(FVector::ZeroVector)
	, TimeWhenPositionLastUpdated(0.0f)
{
}

#if ENGINE_MINOR_VERSION <= 20
void USpatialActorChannel::Init(UNetConnection* InConnection, int32 ChannelIndex, bool bOpenedLocally)
{
	Super::Init(InConnection, ChannelIndex, bOpenedLocally);

	NetDriver = Cast<USpatialNetDriver>(Connection->Driver);
	check(NetDriver);
	Sender = NetDriver->Sender;
	Receiver = NetDriver->Receiver;
}
#else
void USpatialActorChannel::Init(UNetConnection* InConnection, int32 ChannelIndex, EChannelCreateFlags CreateFlag)
{
	Super::Init(InConnection, ChannelIndex, CreateFlag);

	NetDriver = Cast<USpatialNetDriver>(Connection->Driver);
	check(NetDriver);
	Sender = NetDriver->Sender;
	Receiver = NetDriver->Receiver;
}
#endif

void USpatialActorChannel::DeleteEntityIfAuthoritative()
{
	if (NetDriver->Connection == nullptr)
	{
		return;
	}

	bool bHasAuthority = NetDriver->IsAuthoritativeDestructionAllowed() && NetDriver->StaticComponentView->GetAuthority(EntityId, SpatialGDK::Position::ComponentId) == WORKER_AUTHORITY_AUTHORITATIVE;

	UE_LOG(LogSpatialActorChannel, Log, TEXT("Delete entity request on %lld. Has authority: %d"), EntityId, (int)bHasAuthority);

	if (bHasAuthority)
	{
		// Workaround to delay the delete entity request if tearing off.
		// Task to improve this: https://improbableio.atlassian.net/browse/UNR-841
		if (Actor->GetTearOff())
		{
			NetDriver->DelayedSendDeleteEntityRequest(EntityId, 1.0f);
			// Since the entity deletion is delayed, this creates a situation,
			// when the Actor is torn off, but still replicates. 
			// Disabling replication makes RPC calls impossible for this Actor.
			Actor->SetReplicates(false);
		}
		else
		{
			Sender->SendDeleteEntityRequest(EntityId);
		}
	}

	Receiver->CleanupDeletedEntity(EntityId);
}

bool USpatialActorChannel::IsSingletonEntity()
{
	return NetDriver->GlobalStateManager->IsSingletonEntity(EntityId);
}

#if ENGINE_MINOR_VERSION <= 20
bool USpatialActorChannel::CleanUp(const bool bForDestroy)
#else
bool USpatialActorChannel::CleanUp(const bool bForDestroy, EChannelCloseReason CloseReason)
#endif
{
#if WITH_EDITOR
	if (NetDriver != nullptr)
	{
		const bool bDeleteDynamicEntities = GetDefault<ULevelEditorPlaySettings>()->GetDeleteDynamicEntities();

		if (bDeleteDynamicEntities &&
			NetDriver->IsServer() &&
			NetDriver->GetActorChannelByEntityId(EntityId) != nullptr)
		{
			// If we're a server worker, and the entity hasn't already been cleaned up, delete it on shutdown.
			DeleteEntityIfAuthoritative();
		}
	}
#endif

	// Must cleanup actor and subobjects before UActorChannel::Cleanup as it will clear CreateSubObjects
	Receiver->CleanupDeletedEntity(EntityId);

#if ENGINE_MINOR_VERSION <= 20
	return UActorChannel::CleanUp(bForDestroy);
#else
	return UActorChannel::CleanUp(bForDestroy, CloseReason);
#endif
}

#if ENGINE_MINOR_VERSION <= 20
int64 USpatialActorChannel::Close()
{
	DeleteEntityIfAuthoritative();
	return Super::Close();
}
#else
int64 USpatialActorChannel::Close(EChannelCloseReason Reason)
{
	DeleteEntityIfAuthoritative();
	return Super::Close(Reason);
}
#endif

bool USpatialActorChannel::IsDynamicArrayHandle(UObject* Object, uint16 Handle)
{
	check(ObjectHasReplicator(Object));
	FObjectReplicator& Replicator = FindOrCreateReplicator(Object).Get();
	TSharedPtr<FRepLayout>& RepLayout = Replicator.RepLayout;
	check(Handle - 1 < RepLayout->BaseHandleToCmdIndex.Num());
	return RepLayout->Cmds[RepLayout->BaseHandleToCmdIndex[Handle - 1].CmdIndex].Type == ERepLayoutCmdType::DynamicArray;
}

void USpatialActorChannel::UpdateShadowData()
{
	check(Actor);

	// If this channel was responsible for creating the channel, we do not want to initialize our shadow data
	// to the latest state since there could have been state that has changed between creation of the entity
	// and gaining of authority. Revisit this with UNR-1034
	// TODO: UNR-1029 - log when the shadow data differs from the current state of the Actor.
	if (bCreatedEntity)
	{
		return;
	}

	// Refresh shadow data when crossing over servers to prevent stale/out-of-date data.
	ActorReplicator->RepLayout->InitShadowData(ActorReplicator->ChangelistMgr->GetRepChangelistState()->StaticBuffer, Actor->GetClass(), (uint8*)Actor);

	// Refresh the shadow data for all replicated components of this actor as well.
	for (UActorComponent* ActorComponent : Actor->GetReplicatedComponents())
	{
		FObjectReplicator& ComponentReplicator = FindOrCreateReplicator(ActorComponent).Get();
		ComponentReplicator.RepLayout->InitShadowData(ComponentReplicator.ChangelistMgr->GetRepChangelistState()->StaticBuffer, ActorComponent->GetClass(), (uint8*)ActorComponent);
	}
}

void USpatialActorChannel::UpdateSpatialPositionWithFrequencyCheck()
{
	// Check that there has been a sufficient amount of time since the last update.
	if ((NetDriver->Time - TimeWhenPositionLastUpdated) >= (1.0f / GetDefault<USpatialGDKSettings>()->PositionUpdateFrequency))
	{
		UpdateSpatialPosition();
	}
}

FRepChangeState USpatialActorChannel::CreateInitialRepChangeState(TWeakObjectPtr<UObject> Object)
{
	checkf(Object != nullptr, TEXT("Attempted to create initial rep change state on an object which is null."));
	checkf(!Object->IsPendingKill(), TEXT("Attempted to create initial rep change state on an object which is pending kill. This will fail to create a RepLayout: "), *Object->GetName());

#if ENGINE_MINOR_VERSION <= 20
	FObjectReplicator& Replicator = FindOrCreateReplicator(Object).Get();
#else
	FObjectReplicator& Replicator = FindOrCreateReplicator(Object.Get()).Get();
#endif

	TArray<uint16> InitialRepChanged;

	int32 DynamicArrayDepth = 0;
	const int32 CmdCount = Replicator.RepLayout->Cmds.Num();
	for (uint16 CmdIdx = 0; CmdIdx < CmdCount; ++CmdIdx)
	{
		const auto& Cmd = Replicator.RepLayout->Cmds[CmdIdx];

		InitialRepChanged.Add(Cmd.RelativeHandle);

		if (Cmd.Type == ERepLayoutCmdType::DynamicArray)
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
		else if (Cmd.Type == ERepLayoutCmdType::Return)
		{
			DynamicArrayDepth--;
			checkf(DynamicArrayDepth >= 0 || CmdIdx == CmdCount - 1, TEXT("Encountered erroneous RepLayout"));
		}
	}

	return { InitialRepChanged, *Replicator.RepLayout };
}

FHandoverChangeState USpatialActorChannel::CreateInitialHandoverChangeState(const FClassInfo& ClassInfo)
{
	FHandoverChangeState HandoverChanged;
	for (const FHandoverPropertyInfo& PropertyInfo : ClassInfo.HandoverProperties)
	{
		HandoverChanged.Add(PropertyInfo.Handle);
	}

	return HandoverChanged;
}

int64 USpatialActorChannel::ReplicateActor()
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialActorChannelReplicateActor);

	if (!IsReadyForReplication())
	{
		return 0;
	}
	
	check(Actor);
	check(!Closing);
	check(Connection);
	check(Connection->PackageMap);

	const UWorld* const ActorWorld = Actor->GetWorld();

	// Time how long it takes to replicate this particular actor
	STAT(FScopeCycleCounterUObject FunctionScope(Actor));

	// Create an outgoing bunch (to satisfy some of the functions below).
	FOutBunch Bunch(this, 0);
	if (Bunch.IsError())
	{
		return 0;
	}

	bIsReplicatingActor = true;
	FReplicationFlags RepFlags;

	// Send initial stuff.
	if (bCreatingNewEntity)
	{
		RepFlags.bNetInitial = true;
		// Include changes to Bunch (duplicating existing logic in DataChannel), despite us not using it,
		// since these are passed to the virtual OnSerializeNewActor, whose implementations could use them.
		Bunch.bClose = Actor->bNetTemporary;
		Bunch.bReliable = true; // Net temporary sends need to be reliable as well to force them to retry
	}

	// Here, Unreal would have determined if this connection belongs to this actor's Outer.
	// We don't have this concept when it comes to connections, our ownership-based logic is in the interop layer.
	// Setting this to true, but should not matter in the end.
	RepFlags.bNetOwner = true;

	// If initial, send init data.
	if (RepFlags.bNetInitial && OpenedLocally)
	{
		Actor->OnSerializeNewActor(Bunch);
	}

	RepFlags.bNetSimulated = (Actor->GetRemoteRole() == ROLE_SimulatedProxy);
	RepFlags.bRepPhysics = Actor->ReplicatedMovement.bRepPhysics;
	RepFlags.bReplay = ActorWorld && (ActorWorld->DemoNetDriver == Connection->GetDriver());

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
	if (!bCreatingNewEntity)
	{
		if (GetDefault<USpatialGDKSettings>()->bBatchSpatialPositionUpdates)
		{
			Sender->RegisterChannelForPositionUpdate(this);
		}
		else
		{
			UpdateSpatialPositionWithFrequencyCheck();
		}
	}
	
	// Update the replicated property change list.
	FRepChangelistState* ChangelistState = ActorReplicator->ChangelistMgr->GetRepChangelistState();
	bool bWroteSomethingImportant = false;
#if ENGINE_MINOR_VERSION <= 20
	ActorReplicator->ChangelistMgr->Update(Actor, Connection->Driver->ReplicationFrame, ActorReplicator->RepState->LastCompareIndex, RepFlags, bForceCompareProperties);
#else
	ActorReplicator->ChangelistMgr->Update(ActorReplicator->RepState.Get(), Actor, Connection->Driver->ReplicationFrame, RepFlags, bForceCompareProperties);
#endif

	const int32 PossibleNewHistoryIndex = ActorReplicator->RepState->HistoryEnd % FRepState::MAX_CHANGE_HISTORY;
	FRepChangedHistory& PossibleNewHistoryItem = ActorReplicator->RepState->ChangeHistory[PossibleNewHistoryIndex];
	TArray<uint16>& RepChanged = PossibleNewHistoryItem.Changed;

	// Gather all change lists that are new since we last looked, and merge them all together into a single CL
	for (int32 i = ActorReplicator->RepState->LastChangelistIndex; i < ChangelistState->HistoryEnd; i++)
	{
		const int32 HistoryIndex = i % FRepChangelistState::MAX_CHANGE_HISTORY;
		FRepChangedHistory& HistoryItem = ChangelistState->ChangeHistory[HistoryIndex];
		TArray<uint16> Temp = RepChanged;

		if (HistoryItem.Changed.Num() > 0)
		{
			ActorReplicator->RepLayout->MergeChangeList((uint8*)Actor, HistoryItem.Changed, Temp, RepChanged);
		}
		else
		{
			UE_LOG(LogSpatialActorChannel, Warning, TEXT("EntityId: %lld Actor: %s Changelist with index %d has no changed items"), EntityId, *Actor->GetName(), i);
		}
	}

	ActorReplicator->RepState->LastCompareIndex = ChangelistState->CompareIndex;

	const FClassInfo& Info = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());

	FHandoverChangeState HandoverChangeState;

	if (ActorHandoverShadowData != nullptr)
	{
		HandoverChangeState = GetHandoverChangeList(*ActorHandoverShadowData, Actor);
	}

	// If any properties have changed, send a component update.
	if (bCreatingNewEntity || RepChanged.Num() > 0 || HandoverChangeState.Num() > 0)
	{
		if (bCreatingNewEntity)
		{
			// Need to try replicating all subobjects before entity creation to make sure their respective FObjectReplicator exists
			// so we know what subobjects are relevant for replication when creating the entity.
			Actor->ReplicateSubobjects(this, &Bunch, &RepFlags);

			Sender->SendCreateEntityRequest(this);

			// Since we've tried to create this Actor in Spatial, we no longer have authority over the actor since it hasn't been delegated to us.
			Actor->Role = ROLE_SimulatedProxy;
			Actor->RemoteRole = ROLE_Authority;
		}
		else
		{
			FRepChangeState RepChangeState = { RepChanged, GetObjectRepLayout(Actor) };
			Sender->SendComponentUpdates(Actor, Info, this, &RepChangeState, &HandoverChangeState);
			bInterestDirty = false;
		}

		bWroteSomethingImportant = true;
		if (RepChanged.Num() > 0)
		{
			ActorReplicator->RepState->HistoryEnd++;
		}
	}

	UpdateChangelistHistory(ActorReplicator->RepState);

	ActorReplicator->RepState->LastChangelistIndex = ChangelistState->HistoryEnd;

	if (bCreatingNewEntity)
	{
		bCreatingNewEntity = false;
	}
	else
	{
		FOutBunch DummyOutBunch;

		// Actor::ReplicateSubobjects is overridable and enables the Actor to replicate any subobjects directly, via a
		// call back into SpatialActorChannel::ReplicateSubobject, as well as issues a call to UActorComponent::ReplicateSubobjects
		// on any of its replicating actor components. This allows the component to replicate any of its subobjects directly via
		// the same SpatialActorChannel::ReplicateSubobject.
		bWroteSomethingImportant |= Actor->ReplicateSubobjects(this, &DummyOutBunch, &RepFlags);

		for (auto& SubobjectInfoPair : GetHandoverSubobjects())
		{
			UObject* Subobject = SubobjectInfoPair.Key;
			const FClassInfo& SubobjectInfo = *SubobjectInfoPair.Value;

			// Handover shadow data should already exist for this object. If it doesn't, it must have
			// started replicating after SetChannelActor was called on the owning actor.
			TSharedRef<TArray<uint8>>* SubobjectHandoverShadowData = HandoverShadowDataMap.Find(Subobject);
			if (SubobjectHandoverShadowData == nullptr)
			{
				UE_LOG(LogSpatialActorChannel, Warning, TEXT("EntityId: %lld Actor: %s HandoverShadowData not found for Subobject %s"), EntityId, *Actor->GetName(), *Subobject->GetName());
				continue;
			}

			FHandoverChangeState SubobjectHandoverChangeState = GetHandoverChangeList(SubobjectHandoverShadowData->Get(), Subobject);
			if (SubobjectHandoverChangeState.Num() > 0)
			{
				Sender->SendComponentUpdates(Subobject, SubobjectInfo, this, nullptr, &SubobjectHandoverChangeState);
			}
		}

		// Look for deleted subobjects
		for (auto RepComp = ReplicationMap.CreateIterator(); RepComp; ++RepComp)
		{
#if ENGINE_MINOR_VERSION <= 20
			if (!RepComp.Key().IsValid())
#else
			if (!RepComp.Value()->GetWeakObjectPtr().IsValid())
#endif
			{
				FUnrealObjectRef ObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromNetGUID(RepComp.Value().Get().ObjectNetGUID);
				if (ObjectRef.IsValid())
				{
					Sender->SendRemoveComponent(EntityId, NetDriver->ClassInfoManager->GetClassInfoByComponentId(ObjectRef.Offset));
				}

				RepComp.Value()->CleanUp();
				RepComp.RemoveCurrent();
			}
		}
	}

	// If we evaluated everything, mark LastUpdateTime, even if nothing changed.
	LastUpdateTime = Connection->Driver->Time;

	MemMark.Pop();

	bIsReplicatingActor = false;

	bForceCompareProperties = false;		// Only do this once per frame when set

	return (bWroteSomethingImportant) ? 1 : 0;	// TODO: return number of bits written (UNR-664)
}

void USpatialActorChannel::DynamicallyAttachSubobject(UObject* Object)
{
	// Find out if this is a dynamic subobject or a subobject that is already attached but is now replicated
	FUnrealObjectRef ObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromObject(Object);

	const FClassInfo* Info = nullptr;

	// Subobject that's a part of the CDO by default does not need to be created.
	if (ObjectRef.IsValid())
	{
		Info = &NetDriver->ClassInfoManager->GetOrCreateClassInfoByObject(Object);
	}
	else
	{
		Info = TryResolveNewDynamicSubobjectAndGetClassInfo(Object);

		if (Info == nullptr)
		{
			// This is a failure but there is already a log inside TryResolveNewDynamicSubbojectAndGetClassInfo
			return;
		}
	}

	check(Info != nullptr);

	// Check to see if we already have authority over the subobject to be added
	if (NetDriver->StaticComponentView->HasAuthority(EntityId, Info->SchemaComponents[SCHEMA_Data]))
	{
		Sender->SendAddComponent(this, Object, *Info);
	}
	else
	{
		// If we don't, modify the entity ACL to gain authority.
		PendingDynamicSubobjects.Add(TWeakObjectPtr<UObject>(Object));
		Sender->GainAuthorityThenAddComponent(this, Object, Info);
	}
}

const FClassInfo* USpatialActorChannel::TryResolveNewDynamicSubobjectAndGetClassInfo(UObject* Object)
{
	const FClassInfo* Info = nullptr;

	const FClassInfo& SubobjectInfo = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Object->GetClass());

	// Find the first ClassInfo relating to a dynamic subobject
	// which has not been used on this entity.
	for (const auto& DynamicSubobjectInfo : SubobjectInfo.DynamicSubobjectInfo)
	{
		if (!NetDriver->PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(EntityId, DynamicSubobjectInfo->SchemaComponents[SCHEMA_Data])).IsValid())
		{
			Info = &DynamicSubobjectInfo.Get();
			break;
		}
	}

	// If all ClassInfos are used up, we error.
	if (Info == nullptr)
	{
		UE_LOG(LogSpatialActorChannel, Error, TEXT("Too many dynamic subobjects of type %s attached to Actor %s! Please increase"
			" the max number of dynamically attached subobjects per class in the SpatialOS runtime settings."), *Object->GetClass()->GetName(), *Actor->GetName());
		return Info;
	}

	NetDriver->PackageMap->ResolveSubobject(Object, FUnrealObjectRef(EntityId, Info->SchemaComponents[SCHEMA_Data]));

	return Info;
}

bool USpatialActorChannel::ReplicateSubobject(UObject* Object, const FReplicationFlags& RepFlags)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialActorChannelReplicateSubobject);

	bool bCreatedReplicator = false;

#if ENGINE_MINOR_VERSION <= 20
	bCreatedReplicator = !ReplicationMap.Contains(Object);
	FObjectReplicator& Replicator = FindOrCreateReplicator(Object).Get();
#else
	FObjectReplicator& Replicator = FindOrCreateReplicator(Object, &bCreatedReplicator).Get();
#endif

	// If we're creating an entity, don't try replicating 
	if (bCreatingNewEntity)
	{
		return false;
	}

	// New subobject that hasn't been replicated before
	if (bCreatedReplicator)
	{
		// Attach to to the entity
		DynamicallyAttachSubobject(Object);
		return false;
	}

	if (PendingDynamicSubobjects.Contains(Object))
	{
		// Still waiting on subobject to be attached so don't replicate
		return false;
	}

	FRepChangelistState* ChangelistState = Replicator.ChangelistMgr->GetRepChangelistState();
#if ENGINE_MINOR_VERSION <= 20
	Replicator.ChangelistMgr->Update(Object, Replicator.Connection->Driver->ReplicationFrame, Replicator.RepState->LastCompareIndex, RepFlags, bForceCompareProperties);
#else
	Replicator.ChangelistMgr->Update(Replicator.RepState.Get(), Object, Replicator.Connection->Driver->ReplicationFrame, RepFlags, bForceCompareProperties);
#endif

	const int32 PossibleNewHistoryIndex = Replicator.RepState->HistoryEnd % FRepState::MAX_CHANGE_HISTORY;
	FRepChangedHistory& PossibleNewHistoryItem = Replicator.RepState->ChangeHistory[PossibleNewHistoryIndex];
	TArray<uint16>& RepChanged = PossibleNewHistoryItem.Changed;

	// Gather all change lists that are new since we last looked, and merge them all together into a single CL
	for (int32 i = Replicator.RepState->LastChangelistIndex; i < ChangelistState->HistoryEnd; i++)
	{
		const int32 HistoryIndex = i % FRepChangelistState::MAX_CHANGE_HISTORY;
		FRepChangedHistory& HistoryItem = ChangelistState->ChangeHistory[HistoryIndex];
		TArray<uint16> Temp = RepChanged;

		if (HistoryItem.Changed.Num() > 0)
		{
			Replicator.RepLayout->MergeChangeList((uint8*)Object, HistoryItem.Changed, Temp, RepChanged);
		}
		else
		{
			UE_LOG(LogSpatialActorChannel, Warning, TEXT("EntityId: %lld Actor: %s Subobject: %s Changelist with index %d has no changed items"), EntityId, *Actor->GetName(), *Object->GetName(), i);
		}
	}

	Replicator.RepState->LastCompareIndex = ChangelistState->CompareIndex;

	if (RepChanged.Num() > 0)
	{
		FRepChangeState RepChangeState = { RepChanged, GetObjectRepLayout(Object) };

		FUnrealObjectRef ObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromObject(Object);
		if (!ObjectRef.IsValid())
		{
			UE_LOG(LogSpatialActorChannel, Verbose, TEXT("Attempted to replicate an invalid ObjectRef. This may be a dynamic component that couldn't attach: %s"), *Object->GetName());
			return false;
		}
		
		const FClassInfo& Info = NetDriver->ClassInfoManager->GetOrCreateClassInfoByObject(Object);
		Sender->SendComponentUpdates(Object, Info, this, &RepChangeState, nullptr);

		Replicator.RepState->HistoryEnd++;
	}

	UpdateChangelistHistory(Replicator.RepState);
	Replicator.RepState->LastChangelistIndex = ChangelistState->HistoryEnd;

	return RepChanged.Num() > 0;
}

bool USpatialActorChannel::ReplicateSubobject(UObject* Obj, FOutBunch& Bunch, const FReplicationFlags& RepFlags)
{
	// Intentionally don't call Super::ReplicateSubobject() but rather call our custom version instead.
	return ReplicateSubobject(Obj, RepFlags);
}

TMap<UObject*, const FClassInfo*> USpatialActorChannel::GetHandoverSubobjects()
{
	const FClassInfo& Info = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());

	TMap<UObject*, const FClassInfo*> FoundSubobjects;

	for (auto& SubobjectInfoPair : Info.SubobjectInfo)
	{
		const FClassInfo& SubobjectInfo = SubobjectInfoPair.Value.Get();

		if (SubobjectInfo.HandoverProperties.Num() == 0)
		{
			// Not interested in this component if it has no handover properties
			continue;
		}

		UObject* Object = nullptr;
		if (EntityId == 0)
		{
			Object = Actor->GetDefaultSubobjectByName(SubobjectInfo.SubobjectName);
		}
		else
		{
			Object = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(EntityId, SubobjectInfoPair.Key)).Get();
		}


		if (Object == nullptr)
		{
			continue;
		}

		FoundSubobjects.Add(Object, &SubobjectInfo);
	}

	return FoundSubobjects;
}

void USpatialActorChannel::InitializeHandoverShadowData(TArray<uint8>& ShadowData, UObject* Object)
{
	const FClassInfo& ClassInfo = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Object->GetClass());

	uint32 Size = 0;
	for (const FHandoverPropertyInfo& PropertyInfo : ClassInfo.HandoverProperties)
	{
		if (PropertyInfo.ArrayIdx == 0) // For static arrays, the first element will handle the whole array
		{
			// Make sure we conform to Unreal's alignment requirements; this is matched below and in ReplicateActor()
			Size = Align(Size, PropertyInfo.Property->GetMinAlignment());
			Size += PropertyInfo.Property->GetSize();
		}
	}
	ShadowData.AddZeroed(Size);
	uint32 Offset = 0;
	for (const FHandoverPropertyInfo& PropertyInfo : ClassInfo.HandoverProperties)
	{
		if (PropertyInfo.ArrayIdx == 0)
		{
			Offset = Align(Offset, PropertyInfo.Property->GetMinAlignment());
			PropertyInfo.Property->InitializeValue(ShadowData.GetData() + Offset);
			Offset += PropertyInfo.Property->GetSize();
		}
	}
}

FHandoverChangeState USpatialActorChannel::GetHandoverChangeList(TArray<uint8>& ShadowData, UObject* Object)
{
	FHandoverChangeState HandoverChanged;

	const FClassInfo& ClassInfo = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Object->GetClass());

	uint32 ShadowDataOffset = 0;
	for (const FHandoverPropertyInfo& PropertyInfo : ClassInfo.HandoverProperties)
	{
		ShadowDataOffset = Align(ShadowDataOffset, PropertyInfo.Property->GetMinAlignment());

		const uint8* Data = (uint8*)Object + PropertyInfo.Offset;
		uint8* StoredData = ShadowData.GetData() + ShadowDataOffset;
		// Compare and assign.
		if (bCreatingNewEntity || !PropertyInfo.Property->Identical(StoredData, Data))
		{
			HandoverChanged.Add(PropertyInfo.Handle);
			PropertyInfo.Property->CopySingleValue(StoredData, Data);
		}
		ShadowDataOffset += PropertyInfo.Property->ElementSize;
	}

	return HandoverChanged;
}

void USpatialActorChannel::SetChannelActor(AActor* InActor)
{
	Super::SetChannelActor(InActor);
	
	USpatialPackageMapClient* PackageMap = NetDriver->PackageMap;
	EntityId = PackageMap->GetEntityIdFromObject(InActor);

	// If the entity registry has no entry for this actor, this means we need to create it.
	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		bCreatingNewEntity = true;
		TryResolveActor();
	}
	else
	{
		UE_LOG(LogSpatialActorChannel, Verbose, TEXT("Opened channel for actor %s with existing entity ID %lld."), *InActor->GetName(), EntityId);

		if (PackageMap->IsEntityIdPendingCreation(EntityId))
		{
			bCreatingNewEntity = true;
			PackageMap->RemovePendingCreationEntityId(EntityId);
		}
		NetDriver->AddActorChannel(EntityId, this);
	}

	// Set up the shadow data for the handover properties. This is used later to compare the properties and send only changed ones.
	check(!HandoverShadowDataMap.Contains(InActor));

	// Create the shadow map, and store a quick access pointer to it
	const FClassInfo& Info = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(InActor->GetClass());
	if (Info.SchemaComponents[SCHEMA_Handover] != SpatialConstants::INVALID_COMPONENT_ID)
	{
		ActorHandoverShadowData = &HandoverShadowDataMap.Add(InActor, MakeShared<TArray<uint8>>()).Get();
		InitializeHandoverShadowData(*ActorHandoverShadowData, InActor);
	}

	for (auto& SubobjectInfoPair : GetHandoverSubobjects())
	{
		UObject* Subobject = SubobjectInfoPair.Key;

		check(!HandoverShadowDataMap.Contains(Subobject));
		InitializeHandoverShadowData(HandoverShadowDataMap.Add(Subobject, MakeShared<TArray<uint8>>()).Get(), Subobject);
	}

	SavedOwnerWorkerAttribute = SpatialGDK::GetOwnerWorkerAttribute(InActor);
}

bool USpatialActorChannel::TryResolveActor()
{
	EntityId = NetDriver->PackageMap->AllocateEntityIdAndResolveActor(Actor);

	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		return false;
	}
	
	// If a Singleton was created, update the GSM with the proper Id.
	if (Actor->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
	{
		NetDriver->GlobalStateManager->UpdateSingletonEntityId(Actor->GetClass()->GetPathName(), EntityId);
	}

	// Inform USpatialNetDriver of this new actor channel/entity pairing
	NetDriver->AddActorChannel(EntityId, this);

	return true;
}

FObjectReplicator& USpatialActorChannel::PreReceiveSpatialUpdate(UObject* TargetObject)
{
	FNetworkGUID ObjectNetGUID = Connection->Driver->GuidCache->GetOrAssignNetGUID(TargetObject);
	check(!ObjectNetGUID.IsDefault() && ObjectNetGUID.IsValid());

	FObjectReplicator& Replicator = FindOrCreateReplicator(TargetObject).Get();
	TargetObject->PreNetReceive();
	Replicator.RepLayout->InitShadowData(Replicator.RepState->StaticBuffer, TargetObject->GetClass(), (uint8*)TargetObject);

	return Replicator;
}

void USpatialActorChannel::PostReceiveSpatialUpdate(UObject* TargetObject, const TArray<UProperty*>& RepNotifies)
{
	FNetworkGUID ObjectNetGUID = Connection->Driver->GuidCache->GetOrAssignNetGUID(TargetObject);
	check(!ObjectNetGUID.IsDefault() && ObjectNetGUID.IsValid())

	FObjectReplicator& Replicator = FindOrCreateReplicator(TargetObject).Get();
	TargetObject->PostNetReceive();

#if ENGINE_MINOR_VERSION <= 20
	Replicator.RepNotifies = RepNotifies;
#else
	Replicator.RepState->RepNotifies = RepNotifies;
#endif

	Replicator.CallRepNotifies(false);

	if (!TargetObject->IsPendingKill())
	{
		TargetObject->PostRepNotifies();
	}
}

void USpatialActorChannel::OnCreateEntityResponse(const Worker_CreateEntityResponseOp& Op)
{
	check(NetDriver->GetNetMode() < NM_Client);

	if (Actor == nullptr || Actor->IsPendingKill())
	{
		UE_LOG(LogSpatialActorChannel, Warning, TEXT("Actor is invalid after trying to create entity"));
		return;
	}

	if (Op.status_code != WORKER_STATUS_CODE_SUCCESS)
	{
		// UNR-630 - Temporary hack to avoid failure to create entities due to timeout on large maps
		if (Op.status_code == WORKER_STATUS_CODE_TIMEOUT)
		{
			UE_LOG(LogSpatialActorChannel, Warning, TEXT("Failed to create entity for actor %s Reason: %s. Retrying..."), *Actor->GetName(), UTF8_TO_TCHAR(Op.message));
			Sender->SendCreateEntityRequest(this);
		}
		else
		{
			UE_LOG(LogSpatialActorChannel, Error, TEXT("Failed to create entity for actor %s: Reason: %s"), *Actor->GetName(), UTF8_TO_TCHAR(Op.message));
		}

		return;
	}

	bCreatedEntity = true;
	UE_LOG(LogSpatialActorChannel, Verbose, TEXT("Created entity (%lld) for: %s."), Op.entity_id, *Actor->GetName());
}

void USpatialActorChannel::UpdateSpatialPosition()
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialActorChannelUpdateSpatialPosition);

	// Additional check to validate Actor is still present
	if (Actor == nullptr || Actor->IsPendingKill())
	{
		return;
	}

	// When we update an Actor's position, we want to update the position of all the children of this Actor.
	// If this Actor is a PlayerController, we want to update all of its children and its possessed Pawn.
	// That means if this Actor has an Owner or has a NetConnection and is NOT a PlayerController
	// we want to defer updating position until we reach the highest parent.
	AActor* ActorOwner = Actor->GetOwner();

	if ((ActorOwner != nullptr || Actor->GetNetConnection() != nullptr) && !Actor->IsA<APlayerController>())
	{
		// If this Actor's owner is not replicated (e.g. parent = AI Controller), the actor will not have it's spatial
		// position updated as this code will never be run for the parent. 
		if (!(Actor->GetNetConnection() == nullptr && ActorOwner != nullptr && !ActorOwner->GetIsReplicated()))
		{
			return;
		}
	}

	// Check that the Actor has moved sufficiently far to be updated
	const float SpatialPositionThresholdSquared = FMath::Square(GetDefault<USpatialGDKSettings>()->PositionDistanceThreshold);
	FVector ActorSpatialPosition = GetActorSpatialPosition(Actor);
	if (FVector::DistSquared(ActorSpatialPosition, LastPositionSinceUpdate) < SpatialPositionThresholdSquared)
	{
		return;
	}

	LastPositionSinceUpdate = ActorSpatialPosition;
	TimeWhenPositionLastUpdated = NetDriver->Time;

	SendPositionUpdate(Actor, EntityId, LastPositionSinceUpdate);

	if (APlayerController* PlayerController = Cast<APlayerController>(Actor))
	{
		if (APawn* Pawn = PlayerController->GetPawn())
		{
			SendPositionUpdate(Pawn, NetDriver->PackageMap->GetEntityIdFromObject(Pawn), LastPositionSinceUpdate);
		}
	}
}

void USpatialActorChannel::SendPositionUpdate(AActor* InActor, Worker_EntityId InEntityId, const FVector& NewPosition)
{
	if (InEntityId != SpatialConstants::INVALID_ENTITY_ID && NetDriver->StaticComponentView->HasAuthority(InEntityId, SpatialConstants::POSITION_COMPONENT_ID))
	{
		Sender->SendPositionUpdate(InEntityId, NewPosition);
	}

	for (const auto& Child : InActor->Children)
	{
		SendPositionUpdate(Child, NetDriver->PackageMap->GetEntityIdFromObject(Child), NewPosition);
	}
}

FVector USpatialActorChannel::GetActorSpatialPosition(AActor* InActor)
{
	FVector Location = FVector::ZeroVector;

	// If the Actor is a Controller, use its Pawn's position,
	// Otherwise if the Actor has an Owner, use its position.
	// Otherwise if the Actor has a well defined location then use that
	// Otherwise use the origin
	AController* Controller = Cast<AController>(InActor);
	if (Controller != nullptr && Controller->GetPawn() != nullptr)
	{
		USceneComponent* PawnRootComponent = Controller->GetPawn()->GetRootComponent();
		Location = PawnRootComponent ? PawnRootComponent->GetComponentLocation() : FVector::ZeroVector;
	}
	else if (InActor->GetOwner() != nullptr && InActor->GetIsReplicated())
	{
		return GetActorSpatialPosition(InActor->GetOwner());
	}
	else if (USceneComponent* RootComponent = InActor->GetRootComponent())
	{
		Location = RootComponent->GetComponentLocation();
	}

	// Rebase location onto zero origin so actor is positioned correctly in SpatialOS.
	return FRepMovement::RebaseOntoZeroOrigin(Location, InActor);
}

void USpatialActorChannel::RemoveRepNotifiesWithUnresolvedObjs(TArray<UProperty*>& RepNotifies, const FRepLayout& RepLayout, const FObjectReferencesMap& RefMap, UObject* Object)
{
	// Prevent rep notify callbacks from being issued when unresolved obj references exist inside UStructs.
	// This prevents undefined behaviour when engine rep callbacks are issued where they don't expect unresolved objects in native flow.
	RepNotifies.RemoveAll([&](UProperty* Property)
	{
		for (auto& ObjRef : RefMap)
		{
			// ParentIndex will be -1 for handover properties.
			if (ObjRef.Value.ParentIndex < 0)
			{
				continue;
			}

			bool bIsSameRepNotify = RepLayout.Parents[ObjRef.Value.ParentIndex].Property == Property;
			bool bIsArray = RepLayout.Parents[ObjRef.Value.ParentIndex].Property->ArrayDim > 1;
			if (bIsSameRepNotify && !bIsArray)
			{
				UE_LOG(LogSpatialActorChannel, Verbose, TEXT("RepNotify %s on %s ignored due to unresolved Actor"), *Property->GetName(), *Object->GetName());
				return true;
			}
		}
		return false;
	});
}

void USpatialActorChannel::ServerProcessOwnershipChange()
{
	if (!IsAuthoritativeServer())
	{
		return;
	}

	FString NewOwnerWorkerAttribute = SpatialGDK::GetOwnerWorkerAttribute(Actor);

	if (SavedOwnerWorkerAttribute != NewOwnerWorkerAttribute)
	{
		bool bSuccess = Sender->UpdateEntityACLs(EntityId, NewOwnerWorkerAttribute);

		if (bSuccess)
		{
			SavedOwnerWorkerAttribute = NewOwnerWorkerAttribute;
		}
	}
}

void USpatialActorChannel::ClientProcessOwnershipChange(bool bNewNetOwned)
{
	if (bNewNetOwned != bNetOwned)
	{
		bNetOwned = bNewNetOwned;
		Sender->SendComponentInterestForActor(this, GetEntityId(), bNetOwned);
	}
}
