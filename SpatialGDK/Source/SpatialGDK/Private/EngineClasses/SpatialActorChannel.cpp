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

#include "EngineStats.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/GlobalStateManager.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/ClientRPCEndpointLegacy.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/ServerRPCEndpointLegacy.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/GDKPropertyMacros.h"
#include "Utils/RepLayoutUtils.h"
#include "Utils/SpatialActorUtils.h"

DEFINE_LOG_CATEGORY(LogSpatialActorChannel);

DECLARE_CYCLE_STAT(TEXT("ReplicateActor"), STAT_SpatialActorChannelReplicateActor, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("UpdateSpatialPosition"), STAT_SpatialActorChannelUpdateSpatialPosition, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("ReplicateSubobject"), STAT_SpatialActorChannelReplicateSubobject, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("ServerProcessOwnershipChange"), STAT_ServerProcessOwnershipChange, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("ClientProcessOwnershipChange"), STAT_ClientProcessOwnershipChange, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("CallUpdateEntityACLs"), STAT_CallUpdateEntityACLs, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("OnUpdateEntityACLSuccess"), STAT_OnUpdateEntityACLSuccess, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("IsAuthoritativeServer"), STAT_IsAuthoritativeServer, STATGROUP_SpatialNet);

namespace
{
const int32 MaxSendingChangeHistory = FSendingRepState::MAX_CHANGE_HISTORY;

// This is a bookkeeping function that is similar to the one in RepLayout.cpp, modified for our needs (e.g. no NaKs)
// We can't use the one in RepLayout.cpp because it's private and it cannot account for our approach.
// In this function, we poll for any changes in Unreal properties compared to the last time we replicated this actor.
void UpdateChangelistHistory(TUniquePtr<FRepState>& RepState)
{
	FSendingRepState* SendingRepState = RepState->GetSendingRepState();

	check(SendingRepState->HistoryEnd >= SendingRepState->HistoryStart);

	const int32 HistoryCount = SendingRepState->HistoryEnd - SendingRepState->HistoryStart;
	check(HistoryCount < MaxSendingChangeHistory);

	for (int32 i = SendingRepState->HistoryStart; i < SendingRepState->HistoryEnd; i++)
	{
		const int32 HistoryIndex = i % MaxSendingChangeHistory;

		FRepChangedHistory & HistoryItem = SendingRepState->ChangeHistory[HistoryIndex];

		// All active history items should contain a change list
		check(HistoryItem.Changed.Num() > 0);

		HistoryItem.Changed.Empty();
		HistoryItem.OutPacketIdRange = FPacketIdRange();
		SendingRepState->HistoryStart++;
	}

	// Remove any tiling in the history markers to keep them from wrapping over time
	const int32 NewHistoryCount = SendingRepState->HistoryEnd - SendingRepState->HistoryStart;

	check(NewHistoryCount <= MaxSendingChangeHistory);

	SendingRepState->HistoryStart = SendingRepState->HistoryStart % MaxSendingChangeHistory;
	SendingRepState->HistoryEnd = SendingRepState->HistoryStart + NewHistoryCount;
}

} // end anonymous namespace

bool FSpatialObjectRepState::MoveMappedObjectToUnmapped_r(const FUnrealObjectRef& ObjRef, FObjectReferencesMap& ObjectReferencesMap)
{
	bool bFoundRef = false;

	for (auto& ObjReferencePair : ObjectReferencesMap)
	{
		FObjectReferences& ObjReferences = ObjReferencePair.Value;

		if (ObjReferences.Array != NULL)
		{
			if (MoveMappedObjectToUnmapped_r(ObjRef, *ObjReferences.Array))
			{
				bFoundRef = true;
			}
			continue;
		}

		if (ObjReferences.MappedRefs.Contains(ObjRef))
		{
			ObjReferences.MappedRefs.Remove(ObjRef);
			ObjReferences.UnresolvedRefs.Add(ObjRef);
			bFoundRef = true;
		}
	}

	return bFoundRef;
}

bool FSpatialObjectRepState::MoveMappedObjectToUnmapped(const FUnrealObjectRef& ObjRef)
{
	if (MoveMappedObjectToUnmapped_r(ObjRef, ReferenceMap))
	{
		UnresolvedRefs.Add(ObjRef);
		return true;
	}
	return false;
}

void FSpatialObjectRepState::GatherObjectRef(TSet<FUnrealObjectRef>& OutReferenced, TSet<FUnrealObjectRef>& OutUnresolved, const FObjectReferences& CurReferences) const
{
	if (CurReferences.Array)
	{
		for (auto const& Entry : *CurReferences.Array)
		{
			GatherObjectRef(OutReferenced, OutUnresolved, Entry.Value);
		}
	}

	OutUnresolved.Append(CurReferences.UnresolvedRefs);

	// Add both kind of references to OutReferenced map.
	// It is simpler to manage the Ref to RepState map that way by not requiring strict partitioning between both sets.
	OutReferenced.Append(CurReferences.UnresolvedRefs);
	OutReferenced.Append(CurReferences.MappedRefs);
}

void FSpatialObjectRepState::UpdateRefToRepStateMap(FObjectToRepStateMap& RepStateMap)
{
	// Inspired by FObjectReplicator::UpdateGuidToReplicatorMap
	UnresolvedRefs.Empty();

	TSet< FUnrealObjectRef > LocalReferencedObj;
	for (auto& Entry : ReferenceMap)
	{
		GatherObjectRef(LocalReferencedObj, UnresolvedRefs, Entry.Value);
	}

	// TODO : Support references in structures updated by deltas. UNR-2556
	// Look for the code iterating over LifetimeCustomDeltaProperties in the equivalent FObjectReplicator method.

	// Go over all referenced guids, and make sure we're tracking them in the GuidToReplicatorMap
	for (const FUnrealObjectRef& Ref : LocalReferencedObj)
	{
		if (!ReferencedObj.Contains(Ref))
		{
			RepStateMap.FindOrAdd(Ref).Add(ThisObj);
		}
	}

	// Remove any guids that we were previously tracking but no longer should
	for (const FUnrealObjectRef& Ref : ReferencedObj)
	{
		if (!LocalReferencedObj.Contains(Ref))
		{
			TSet<FChannelObjectPair>* RepStatesWithRef = RepStateMap.Find(Ref);

			if (ensure(RepStatesWithRef))
			{
				RepStatesWithRef->Remove(ThisObj);

				if (RepStatesWithRef->Num() == 0)
				{
					RepStateMap.Remove(Ref);
				}
			}
		}
	}

	ReferencedObj = MoveTemp(LocalReferencedObj);
}

USpatialActorChannel::USpatialActorChannel(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
	, bCreatedEntity(false)
	, bCreatingNewEntity(false)
	, EntityId(SpatialConstants::INVALID_ENTITY_ID)
	, bInterestDirty(false)
	, bNetOwned(false)
	, NetDriver(nullptr)
	, LastPositionSinceUpdate(FVector::ZeroVector)
	, TimeWhenPositionLastUpdated(0.0)
{
}

void USpatialActorChannel::Init(UNetConnection* InConnection, int32 ChannelIndex, EChannelCreateFlags CreateFlag)
{
	Super::Init(InConnection, ChannelIndex, CreateFlag);

	// Actor Channels are pooled, so we must initialize internal state here.
	bCreatedEntity = false;
	bCreatingNewEntity = false;
	EntityId = SpatialConstants::INVALID_ENTITY_ID;
	bInterestDirty = false;
	bNetOwned = false;
	bIsAuthClient = false;
	bIsAuthServer = false;
	LastPositionSinceUpdate = FVector::ZeroVector;
	TimeWhenPositionLastUpdated = 0.0;
	AuthorityReceivedTimestamp = 0;
	bNeedOwnerInterestUpdate = false;

	PendingDynamicSubobjects.Empty();
	SavedConnectionOwningWorkerId.Empty();
	SavedInterestBucketComponentID = SpatialConstants::INVALID_COMPONENT_ID;

	FramesTillDormancyAllowed = 0;

	ActorHandoverShadowData = nullptr;
	HandoverShadowDataMap.Empty();

	NetDriver = Cast<USpatialNetDriver>(Connection->Driver);
	check(NetDriver);
	Sender = NetDriver->Sender;
	Receiver = NetDriver->Receiver;
}

void USpatialActorChannel::RetireEntityIfAuthoritative()
{
	if (NetDriver->Connection == nullptr)
	{
		return;
	}

	if (!NetDriver->IsAuthoritativeDestructionAllowed())
	{
		return;
	}

	const bool bHasAuthority = NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialGDK::Position::ComponentId);
	if (Actor != nullptr)
	{
		if (bHasAuthority)
		{
			// Workaround to delay the delete entity request if tearing off.
			// Task to improve this: UNR-841
			if (Actor->GetTearOff())
			{
				NetDriver->DelayedRetireEntity(EntityId, 1.0f, Actor->IsNetStartupActor());
				// Since the entity deletion is delayed, this creates a situation,
				// when the Actor is torn off, but still replicates.
				// Disabling replication makes RPC calls impossible for this Actor.
				Actor->SetReplicates(false);
			}
			else
			{
				Sender->RetireEntity(EntityId, Actor->IsNetStartupActor());
			}
		}
		else if (bCreatedEntity) // We have not gained authority yet
		{
			Actor->SetReplicates(false);
			Receiver->RetireWhenAuthoritive(EntityId, NetDriver->ClassInfoManager->GetComponentIdForClass(*Actor->GetClass()), Actor->IsNetStartupActor(), Actor->GetTearOff()); // Ensure we don't recreate the actor
		}
	}
	else
	{
		// This is unsupported, and shouldn't happen, don't attempt to cleanup entity to better indicate something has gone wrong
		UE_LOG(LogSpatialActorChannel, Error, TEXT("RetireEntityIfAuthoritative called on actor channel with null actor - entity id (%lld)"), EntityId);
	}
}

bool USpatialActorChannel::CleanUp(const bool bForDestroy, EChannelCloseReason CloseReason)
{
	if (NetDriver != nullptr)
	{
#if WITH_EDITOR
		const bool bDeleteDynamicEntities = GetDefault<ULevelEditorPlaySettings>()->GetDeleteDynamicEntities();

		if (bDeleteDynamicEntities &&
			NetDriver->IsServer() &&
			NetDriver->GetActorChannelByEntityId(EntityId) != nullptr &&
			CloseReason != EChannelCloseReason::Dormancy)
		{
			// If we're a server worker, and the entity hasn't already been cleaned up, delete it on shutdown.
			RetireEntityIfAuthoritative();
		}
#endif // WITH_EDITOR

		if (CloseReason != EChannelCloseReason::Dormancy)
		{
			// Must cleanup actor and subobjects before UActorChannel::Cleanup as it will clear CreateSubObjects.
			NetDriver->PackageMap->RemoveEntityActor(EntityId);
		}
		else
		{
			NetDriver->RegisterDormantEntityId(EntityId);
		}

		if (CloseReason == EChannelCloseReason::Destroyed || CloseReason == EChannelCloseReason::LevelUnloaded)
		{
			Receiver->ClearPendingRPCs(EntityId);
			Sender->ClearPendingRPCs(EntityId);
		}
		NetDriver->RemoveActorChannel(EntityId, *this);
	}


	return UActorChannel::CleanUp(bForDestroy, CloseReason);
}

int64 USpatialActorChannel::Close(EChannelCloseReason Reason)
{
	if (Reason == EChannelCloseReason::Dormancy)
	{
		// Closed for dormancy reasons, ensure we update the component state of this entity.
		const bool bMakeDormant = true;
		NetDriver->RefreshActorDormancy(Actor, bMakeDormant);
		NetDriver->RegisterDormantEntityId(EntityId);
	}
	else if (Reason == EChannelCloseReason::Relevancy)
	{
		check(IsAuthoritativeServer());
		// Do nothing except close actor channel - this should only get processed on auth server
	}
	else
	{
		RetireEntityIfAuthoritative();
		NetDriver->PackageMap->RemoveEntityActor(EntityId);
	}

	NetDriver->RemoveActorChannel(EntityId, *this);

	return Super::Close(Reason);
}

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

	// If this channel was responsible for creating the actor, we do not want to initialize our shadow data
	// to the latest state since there could have been state that has changed between creation of the entity
	// and gaining of authority. Revisit this with UNR-1034
	// TODO: UNR-1029 - log when the shadow data differs from the current state of the Actor.
	if (bCreatedEntity)
	{
		return;
	}

	// Refresh shadow data when crossing over servers to prevent stale/out-of-date data.
	ResetShadowData(*ActorReplicator->RepLayout, ActorReplicator->ChangelistMgr->GetRepChangelistState()->StaticBuffer, Actor);

	// Refresh the shadow data for all replicated components of this actor as well.
	for (UActorComponent* ActorComponent : Actor->GetReplicatedComponents())
	{
		FObjectReplicator& ComponentReplicator = FindOrCreateReplicator(ActorComponent).Get();
		ResetShadowData(*ComponentReplicator.RepLayout, ComponentReplicator.ChangelistMgr->GetRepChangelistState()->StaticBuffer, ActorComponent);
	}
}

void USpatialActorChannel::UpdateSpatialPositionWithFrequencyCheck()
{
	// Check that there has been a sufficient amount of time since the last update.
	if ((NetDriver->GetElapsedTime() - TimeWhenPositionLastUpdated) >= (1.0f / GetDefault<USpatialGDKSettings>()->PositionUpdateFrequency))
	{
		UpdateSpatialPosition();
	}
}

FRepChangeState USpatialActorChannel::CreateInitialRepChangeState(TWeakObjectPtr<UObject> Object)
{
	checkf(Object != nullptr, TEXT("Attempted to create initial rep change state on an object which is null."));
	checkf(!Object->IsPendingKill(), TEXT("Attempted to create initial rep change state on an object which is pending kill. This will fail to create a RepLayout: "), *Object->GetName());

	FObjectReplicator& Replicator = FindOrCreateReplicator(Object.Get()).Get();

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

#if STATS
	// Group specific actor class stats by parent native class, which is what vanilla Unreal does.
	UClass* ParentNativeClass = GetParentNativeClass(Actor->GetClass());
	SCOPE_CYCLE_UOBJECT(ParentNativeClass, ParentNativeClass);
#endif

	// Group actors by exact class, one level below parent native class.
	SCOPE_CYCLE_UOBJECT(ReplicateActor, Actor);

	const bool bReplay = ActorWorld && ActorWorld->DemoNetDriver == Connection->GetDriver();

	//////////////////////////////////////////////////////////////////////////
	// Begin - error and stat duplication from DataChannel::ReplicateActor()
	if (!bReplay)
	{
		GNumReplicateActorCalls++;
	}

	// triggering replication of an Actor while already in the middle of replication can result in invalid data being sent and is therefore illegal
	if (bIsReplicatingActor)
	{
		FString Error(FString::Printf(TEXT("ReplicateActor called while already replicating! %s"), *Describe()));
		UE_LOG(LogNet, Log, TEXT("%s"), *Error);
		ensureMsgf(false, TEXT("%s"), *Error);
		return 0;
	}
	else if (bActorIsPendingKill)
	{
		// Don't need to do anything, because it should have already been logged.
		return 0;
	}
	// If our Actor is PendingKill, that's bad. It means that somehow it wasn't properly removed
	// from the NetDriver or ReplicationDriver.
	// TODO: Maybe notify the NetDriver / RepDriver about this, and have the channel close?
	else if (Actor->IsPendingKillOrUnreachable())
	{
		bActorIsPendingKill = true;
		ActorReplicator.Reset();
		FString Error(FString::Printf(TEXT("ReplicateActor called with PendingKill Actor! %s"), *Describe()));
		UE_LOG(LogNet, Log, TEXT("%s"), *Error);
		ensureMsgf(false, TEXT("%s"), *Error);
		return 0;
	}
	// End - error and stat duplication from DataChannel::ReplicateActor()
	//////////////////////////////////////////////////////////////////////////

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
#if ENGINE_MINOR_VERSION <= 23
	RepFlags.bRepPhysics = Actor->ReplicatedMovement.bRepPhysics;
#else
	RepFlags.bRepPhysics = Actor->GetReplicatedMovement().bRepPhysics;
#endif
	RepFlags.bReplay = bReplay;

	UE_LOG(LogNetTraffic, Log, TEXT("Replicate %s, bNetInitial: %d, bNetOwner: %d"), *Actor->GetName(), RepFlags.bNetInitial, RepFlags.bNetOwner);

	FMemMark MemMark(FMemStack::Get());	// The calls to ReplicateProperties will allocate memory on FMemStack::Get(), and use it in ::PostSendBunch. we free it below

	// ----------------------------------------------------------
	// Replicate Actor and Component properties and RPCs
	// ----------------------------------------------------------

#if USE_NETWORK_PROFILER
	const uint32 ActorReplicateStartTime = GNetworkProfiler.IsTrackingEnabled() ? FPlatformTime::Cycles() : 0;
#endif

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	// Update SpatialOS position.
	if (!bCreatingNewEntity)
	{
		if (SpatialGDKSettings->bBatchSpatialPositionUpdates)
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

	ActorReplicator->RepLayout->UpdateChangelistMgr(ActorReplicator->RepState->GetSendingRepState(), *ActorReplicator->ChangelistMgr, Actor, Connection->Driver->ReplicationFrame, RepFlags, bForceCompareProperties);
	FSendingRepState* SendingRepState = ActorReplicator->RepState->GetSendingRepState();

	const int32 PossibleNewHistoryIndex = SendingRepState->HistoryEnd % MaxSendingChangeHistory;
	FRepChangedHistory& PossibleNewHistoryItem = SendingRepState->ChangeHistory[PossibleNewHistoryIndex];
	TArray<uint16>& RepChanged = PossibleNewHistoryItem.Changed;

	// Gather all change lists that are new since we last looked, and merge them all together into a single CL
	for (int32 i = SendingRepState->LastChangelistIndex; i < ChangelistState->HistoryEnd; i++)
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

	SendingRepState->LastCompareIndex = ChangelistState->CompareIndex;

	const FClassInfo& Info = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());

	FHandoverChangeState HandoverChangeState;

	if (ActorHandoverShadowData != nullptr)
	{
		HandoverChangeState = GetHandoverChangeList(*ActorHandoverShadowData, Actor);
	}

	ReplicationBytesWritten = 0;

	if (!bCreatingNewEntity && NeedOwnerInterestUpdate())
	{
		AActor* HierarchyRoot = SpatialGDK::GetHierarchyRoot(Actor);
		Worker_EntityId OwnerId = NetDriver->PackageMap->GetEntityIdFromObject(HierarchyRoot);

		if (HierarchyRoot == nullptr || OwnerId != SpatialConstants::INVALID_ENTITY_ID)
		{
			bool bOwnerReady;
			Sender->UpdateInterestComponent(Actor, bOwnerReady);

			SetNeedOwnerInterestUpdate(!bOwnerReady);
		}
	}

	// If any properties have changed, send a component update.
	if (bCreatingNewEntity || RepChanged.Num() > 0 || HandoverChangeState.Num() > 0)
	{
		if (bCreatingNewEntity)
		{
			// Need to try replicating all subobjects before entity creation to make sure their respective FObjectReplicator exists
			// so we know what subobjects are relevant for replication when creating the entity.
			Actor->ReplicateSubobjects(this, &Bunch, &RepFlags);

			Sender->SendCreateEntityRequest(this, ReplicationBytesWritten);

			bCreatedEntity = true;

			// We preemptively set the Actor role to SimulatedProxy if load balancing is disabled
			// (since the legacy behaviour is to wait until Spatial tells us we have authority)
			if (NetDriver->LoadBalanceStrategy == nullptr)
			{
				Actor->Role = ROLE_SimulatedProxy;
				Actor->RemoteRole = ROLE_Authority;
			}
		}
		else
		{
			FRepChangeState RepChangeState = { RepChanged, GetObjectRepLayout(Actor) };

			Sender->SendComponentUpdates(Actor, Info, this, &RepChangeState, &HandoverChangeState, ReplicationBytesWritten);

			bInterestDirty = false;
		}

		if (RepChanged.Num() > 0)
		{
			SendingRepState->HistoryEnd++;
		}
	}

	UpdateChangelistHistory(ActorReplicator->RepState);

	// This would indicate we need to flush our state before we could consider going dormant. In Spatial, this
	// dormancy can occur immediately (because we don't require acking), which means that dormancy can be thrashed
	// on and off if AActor::FlushNetDormancy is being called (possibly because replicated properties are being updated
	// within blueprints which invokes this call). Give a few frames before allowing channel to go dormant.
	if (ActorReplicator->bLastUpdateEmpty == 0)
	{
		FramesTillDormancyAllowed = 2;
	}
	else if (FramesTillDormancyAllowed > 0)
	{
		--FramesTillDormancyAllowed;
	}

	SendingRepState->LastChangelistIndex = ChangelistState->HistoryEnd;
	SendingRepState->bOpenAckedCalled = true;
	ActorReplicator->bLastUpdateEmpty = 1;

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
		Actor->ReplicateSubobjects(this, &DummyOutBunch, &RepFlags);

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
				Sender->SendComponentUpdates(Subobject, SubobjectInfo, this, nullptr, &SubobjectHandoverChangeState, ReplicationBytesWritten);
			}
		}

		// Look for deleted subobjects
		for (auto RepComp = ReplicationMap.CreateIterator(); RepComp; ++RepComp)
		{
			if (!RepComp.Value()->GetWeakObjectPtr().IsValid())
			{
				FUnrealObjectRef ObjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromNetGUID(RepComp.Value().Get().ObjectNetGUID);

				if (ObjectRef.IsValid())
				{
					OnSubobjectDeleted(ObjectRef, RepComp.Key());

					Sender->SendRemoveComponentForClassInfo(EntityId, NetDriver->ClassInfoManager->GetClassInfoByComponentId(ObjectRef.Offset));
				}

				RepComp.Value()->CleanUp();
				RepComp.RemoveCurrent();
			}
		}
	}

#if USE_NETWORK_PROFILER
	NETWORK_PROFILER(GNetworkProfiler.TrackReplicateActor(Actor, RepFlags, FPlatformTime::Cycles() - ActorReplicateStartTime, Connection));
#endif

	// If we evaluated everything, mark LastUpdateTime, even if nothing changed.
	LastUpdateTime = NetDriver->GetElapsedTime();

	MemMark.Pop();

	bIsReplicatingActor = false;

	bForceCompareProperties = false;		// Only do this once per frame when set

	if (ReplicationBytesWritten > 0)
	{
		INC_DWORD_STAT_BY(STAT_NumReplicatedActors, 1);
	}
	INC_DWORD_STAT_BY(STAT_NumReplicatedActorBytes, ReplicationBytesWritten);

	return ReplicationBytesWritten * 8;
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
		Info = NetDriver->PackageMap->TryResolveNewDynamicSubobjectAndGetClassInfo(Object);

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
		Sender->SendAddComponentForSubobject(this, Object, *Info, ReplicationBytesWritten);
	}
	else
	{
		// If we don't, modify the entity ACL to gain authority.
		PendingDynamicSubobjects.Add(TWeakObjectPtr<UObject>(Object));
		Sender->GainAuthorityThenAddComponent(this, Object, Info);
	}
}

bool USpatialActorChannel::IsListening() const
{
	if (NetDriver->IsServer())
	{
		if (SpatialGDK::ClientRPCEndpointLegacy* Endpoint = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::ClientRPCEndpointLegacy>(EntityId))
		{
			return Endpoint->bReady;
		}
	}
	else
	{
		if (SpatialGDK::ServerRPCEndpointLegacy* Endpoint = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::ServerRPCEndpointLegacy>(EntityId))
		{
			return Endpoint->bReady;
		}
	}

	return false;
}

bool USpatialActorChannel::ReplicateSubobject(UObject* Object, const FReplicationFlags& RepFlags)
{
	SCOPE_CYCLE_COUNTER(STAT_SpatialActorChannelReplicateSubobject);

#if STATS
	// Break down the subobject timing stats by parent native class.
	UClass* ParentNativeClass = GetParentNativeClass(Object->GetClass());
	SCOPE_CYCLE_UOBJECT(ReplicateSubobjectParentClass, ParentNativeClass);
#endif

	// Further break down the subobject timing stats by class.
	SCOPE_CYCLE_UOBJECT(ReplicateSubobjectSpecificClass, Object);

	bool bCreatedReplicator = false;

	FObjectReplicator& Replicator = FindOrCreateReplicator(Object, &bCreatedReplicator).Get();

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

	Replicator.RepLayout->UpdateChangelistMgr(Replicator.RepState->GetSendingRepState(), *Replicator.ChangelistMgr, Object, Replicator.Connection->Driver->ReplicationFrame, RepFlags, bForceCompareProperties);
	FSendingRepState* SendingRepState = Replicator.RepState->GetSendingRepState();

	const int32 PossibleNewHistoryIndex = SendingRepState->HistoryEnd % MaxSendingChangeHistory;
	FRepChangedHistory& PossibleNewHistoryItem = SendingRepState->ChangeHistory[PossibleNewHistoryIndex];
	TArray<uint16>& RepChanged = PossibleNewHistoryItem.Changed;

	// Gather all change lists that are new since we last looked, and merge them all together into a single CL
	for (int32 i = SendingRepState->LastChangelistIndex; i < ChangelistState->HistoryEnd; i++)
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

	SendingRepState->LastCompareIndex = ChangelistState->CompareIndex;

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
		Sender->SendComponentUpdates(Object, Info, this, &RepChangeState, nullptr, ReplicationBytesWritten);

		SendingRepState->HistoryEnd++;
	}

	UpdateChangelistHistory(Replicator.RepState);

	SendingRepState->LastChangelistIndex = ChangelistState->HistoryEnd;
	SendingRepState->bOpenAckedCalled = true;
	Replicator.bLastUpdateEmpty = 1;

	return RepChanged.Num() > 0;
}

bool USpatialActorChannel::ReplicateSubobject(UObject* Obj, FOutBunch& Bunch, const FReplicationFlags& RepFlags)
{
	// Intentionally don't call Super::ReplicateSubobject() but rather call our custom version instead.
	return ReplicateSubobject(Obj, RepFlags);
}

bool USpatialActorChannel::ReadyForDormancy(bool bSuppressLogs /*= false*/)
{
	// Check Receiver doesn't have any pending operations for this channel
	if (Receiver->IsPendingOpsOnChannel(*this))
	{
		return false;
	}

	// Hasn't been waiting for dormancy long enough allow dormancy, soft attempt to prevent dormancy thrashing
	if (FramesTillDormancyAllowed > 0)
	{
		return false;
	}

	return Super::ReadyForDormancy(bSuppressLogs);
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

void USpatialActorChannel::SetChannelActor(AActor* InActor, ESetChannelActorFlags Flags)
{
	Super::SetChannelActor(InActor, Flags);
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
		NetDriver->UnregisterDormantEntityId(EntityId);
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

	SavedConnectionOwningWorkerId = SpatialGDK::GetConnectionOwningWorkerId(InActor);
}

bool USpatialActorChannel::TryResolveActor()
{
	EntityId = NetDriver->PackageMap->AllocateEntityIdAndResolveActor(Actor);

	if (EntityId == SpatialConstants::INVALID_ENTITY_ID)
	{
		return false;
	}

	// Inform USpatialNetDriver of this new actor channel/entity pairing
	NetDriver->AddActorChannel(EntityId, this);

	return true;
}

FObjectReplicator* USpatialActorChannel::PreReceiveSpatialUpdate(UObject* TargetObject)
{
	// If there is no NetGUID for this object, we will crash in FObjectReplicator::StartReplicating, so we verify this here.
	FNetworkGUID ObjectNetGUID = Connection->Driver->GuidCache->GetOrAssignNetGUID(TargetObject);
	if (ObjectNetGUID.IsDefault() || !ObjectNetGUID.IsValid())
	{
		// SpatialReceiver tried to resolve this object in the PackageMap, but it didn't propagate to GuidCache.
		// This could happen if the UnrealObjectRef was already mapped to a different object that's been destroyed.
		UE_LOG(LogSpatialActorChannel, Error, TEXT("PreReceiveSpatialUpdate: NetGUID is invalid! Object: %s"), *TargetObject->GetPathName());
		return nullptr;
	}

	FObjectReplicator& Replicator = FindOrCreateReplicator(TargetObject).Get();
	TargetObject->PreNetReceive();

	return &Replicator;
}

void USpatialActorChannel::PostReceiveSpatialUpdate(UObject* TargetObject, const TArray<GDK_PROPERTY(Property)*>& RepNotifies)
{
	FObjectReplicator& Replicator = FindOrCreateReplicator(TargetObject).Get();
	TargetObject->PostNetReceive();

	Replicator.RepState->GetReceivingRepState()->RepNotifies = RepNotifies;

	Replicator.CallRepNotifies(false);
}

void USpatialActorChannel::OnCreateEntityResponse(const Worker_CreateEntityResponseOp& Op)
{
	check(NetDriver->GetNetMode() < NM_Client);

	if (Actor == nullptr || Actor->IsPendingKill())
	{
		UE_LOG(LogSpatialActorChannel, Log, TEXT("Actor is invalid after trying to create entity"));
		return;
	}

	// True if the entity is in the worker's view.
	// If this is the case then we know the entity was created and do not need to retry if the request timed-out.
	const bool bEntityIsInView = NetDriver->StaticComponentView->HasComponent(SpatialGDK::Position::ComponentId, GetEntityId());

	switch (static_cast<Worker_StatusCode>(Op.status_code))
	{
	case WORKER_STATUS_CODE_SUCCESS:
		UE_LOG(LogSpatialActorChannel, Verbose, TEXT("Create entity request succeeded. "
			"Actor %s, request id: %d, entity id: %lld, message: %s"), *Actor->GetName(), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
		break;
	case WORKER_STATUS_CODE_TIMEOUT:
		if (bEntityIsInView)
		{
			UE_LOG(LogSpatialActorChannel, Log, TEXT("Create entity request failed but the entity was already in view. "
				"Actor %s, request id: %d, entity id: %lld, message: %s"), *Actor->GetName(), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
		}
		else
		{
			UE_LOG(LogSpatialActorChannel, Warning, TEXT("Create entity request timed out. Retrying. "
				"Actor %s, request id: %d, entity id: %lld, message: %s"), *Actor->GetName(), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));

			// TODO: UNR-664 - Track these bytes written to use in saturation.
			uint32 BytesWritten = 0;
			Sender->SendCreateEntityRequest(this, BytesWritten);
		}
		break;
	case WORKER_STATUS_CODE_APPLICATION_ERROR:
		if (bEntityIsInView)
		{
			UE_LOG(LogSpatialActorChannel, Log, TEXT("Create entity request failed as the entity already exists and is in view. "
				"Actor %s, request id: %d, entity id: %lld, message: %s"), *Actor->GetName(), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
		}
		else
		{
			UE_LOG(LogSpatialActorChannel, Warning, TEXT("Create entity request failed."
				"Either the reservation expired, the entity already existed, or the entity was invalid. "
				"Actor %s, request id: %d, entity id: %lld, message: %s"), *Actor->GetName(), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
		}
		break;
	default:
		UE_LOG(LogSpatialActorChannel, Error, TEXT("Create entity request failed. This likely indicates a bug in the Unreal GDK and should be reported."
			"Actor %s, request id: %d, entity id: %lld, message: %s"), *Actor->GetName(), Op.request_id, Op.entity_id, UTF8_TO_TCHAR(Op.message));
		break;
	}
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
	FVector ActorSpatialPosition = SpatialGDK::GetActorSpatialPosition(Actor);
	if (FVector::DistSquared(ActorSpatialPosition, LastPositionSinceUpdate) < SpatialPositionThresholdSquared)
	{
		return;
	}

	LastPositionSinceUpdate = ActorSpatialPosition;
	TimeWhenPositionLastUpdated = NetDriver->GetElapsedTime();

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

void USpatialActorChannel::RemoveRepNotifiesWithUnresolvedObjs(TArray<GDK_PROPERTY(Property)*>& RepNotifies, const FRepLayout& RepLayout, const FObjectReferencesMap& RefMap, UObject* Object)
{
	// Prevent rep notify callbacks from being issued when unresolved obj references exist inside UStructs.
	// This prevents undefined behaviour when engine rep callbacks are issued where they don't expect unresolved objects in native flow.
	RepNotifies.RemoveAll([&](GDK_PROPERTY(Property)* Property)
	{
		for (auto& ObjRef : RefMap)
		{
			// ParentIndex will be -1 for handover properties.
			if (ObjRef.Value.ParentIndex < 0)
			{
				continue;
			}

			// Skip only when there are unresolved refs (FObjectReferencesMap entry contains both mapped and unresolved references).
			if (ObjRef.Value.UnresolvedRefs.Num() == 0)
			{
				continue;
			}

			bool bIsSameRepNotify = RepLayout.Parents[ObjRef.Value.ParentIndex].Property == Property;
			bool bIsArray = RepLayout.Parents[ObjRef.Value.ParentIndex].Property->ArrayDim > 1 || GDK_CASTFIELD<GDK_PROPERTY(ArrayProperty)>(Property) != nullptr;
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
	SCOPE_CYCLE_COUNTER(STAT_ServerProcessOwnershipChange);
	{
		if (!IsReadyForReplication()
		|| !IsAuthoritativeServer())
		{
			return;
		}
	}

	// We only want to iterate through child Actors if the connection-owning worker ID or interest bucket component ID
	// for this Actor changes. This bool is used to keep track of whether it has changed, and used to exit early below.
	bool bUpdatedThisActor = false;

	// Changing an Actor's owner can affect its NetConnection so we need to reevaluate this.
	FString NewClientConnectionWorkerId = SpatialGDK::GetConnectionOwningWorkerId(Actor);
	if (SavedConnectionOwningWorkerId != NewClientConnectionWorkerId)
	{
		// Update the NetOwningClientWorker component.
		check(NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID));
		SpatialGDK::NetOwningClientWorker* NetOwningClientWorkerData = NetDriver->StaticComponentView->GetComponentData<SpatialGDK::NetOwningClientWorker>(EntityId);
		NetOwningClientWorkerData->WorkerId = NewClientConnectionWorkerId;
		FWorkerComponentUpdate Update = NetOwningClientWorkerData->CreateNetOwningClientWorkerUpdate();
		NetDriver->Connection->SendComponentUpdate(EntityId, &Update);

		// Update the EntityACL component (if authoritative).
		if (NetDriver->StaticComponentView->HasAuthority(EntityId, SpatialConstants::ENTITY_ACL_COMPONENT_ID))
		{
			Sender->UpdateClientAuthoritativeComponentAclEntries(EntityId, NewClientConnectionWorkerId);
		}

		SavedConnectionOwningWorkerId = NewClientConnectionWorkerId;

		bUpdatedThisActor = true;
	}

	// Owner changed, update the actor's interest over it.
	bool bOwnerReady;
	Sender->UpdateInterestComponent(Actor, bOwnerReady);

	SetNeedOwnerInterestUpdate(!bOwnerReady);

	// Changing owner can affect which interest bucket the Actor should be in so we need to update it.
	Worker_ComponentId NewInterestBucketComponentId = NetDriver->ClassInfoManager->ComputeActorInterestComponentId(Actor);
	if (SavedInterestBucketComponentID != NewInterestBucketComponentId)
	{
		Sender->SendInterestBucketComponentChange(EntityId, SavedInterestBucketComponentID, NewInterestBucketComponentId);

		SavedInterestBucketComponentID = NewInterestBucketComponentId;

		bUpdatedThisActor = true;
	}

	// If we haven't updated this Actor, skip attempting to update child Actors.
	if (!bUpdatedThisActor)
	{
		return;
	}

	// Changes to NetConnection and InterestBucket for an Actor also affect all descendants which we
	// need to iterate through.
	for (AActor* Child : Actor->Children)
	{
		Worker_EntityId ChildEntityId = NetDriver->PackageMap->GetEntityIdFromObject(Child);

		if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(ChildEntityId))
		{
			Channel->ServerProcessOwnershipChange();
		}
	}
}

void USpatialActorChannel::ClientProcessOwnershipChange(bool bNewNetOwned)
{
	SCOPE_CYCLE_COUNTER(STAT_ClientProcessOwnershipChange);
	if (bNewNetOwned != bNetOwned)
	{
		bNetOwned = bNewNetOwned;

		Actor->SetIsOwnedByClient(bNetOwned);

		if (bNetOwned)
		{
			Actor->OnClientOwnershipGained();
		}
		else
		{
			Actor->OnClientOwnershipLost();
		}
	}
}

void USpatialActorChannel::OnSubobjectDeleted(const FUnrealObjectRef& ObjectRef, UObject* Object)
{
	CreateSubObjects.Remove(Object);

	Receiver->MoveMappedObjectToUnmapped(ObjectRef);
	if (FSpatialObjectRepState* SubObjectRefMap = ObjectReferenceMap.Find(Object))
	{
		Receiver->CleanupRepStateMap(*SubObjectRefMap);
		ObjectReferenceMap.Remove(Object);
	}
}

void USpatialActorChannel::ResetShadowData(FRepLayout& RepLayout, FRepStateStaticBuffer& StaticBuffer, UObject* TargetObject)
{
	if (StaticBuffer.Num() == 0)
	{
		RepLayout.InitRepStateStaticBuffer(StaticBuffer, reinterpret_cast<const uint8*>(TargetObject));
	}
	else
	{
		RepLayout.CopyProperties(StaticBuffer, reinterpret_cast<uint8*>(TargetObject));
	}
}
