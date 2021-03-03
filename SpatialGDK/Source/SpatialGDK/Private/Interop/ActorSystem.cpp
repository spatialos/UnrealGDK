// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/ActorSystem.h"

#include "EngineClasses/SpatialFastArrayNetSerialize.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/PlayerState.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/SpatialReceiver.h"
#include "Interop/SpatialSender.h"
#include "Schema/Restricted.h"
#include "SpatialConstants.h"
#include "SpatialView/EntityDelta.h"
#include "SpatialView/SubView.h"
#include "Utils/ComponentReader.h"
#include "Utils/RepLayoutUtils.h"

DEFINE_LOG_CATEGORY(LogActorSystem);

DECLARE_CYCLE_STAT(TEXT("Actor System RemoveEntity"), STAT_ActorSystemRemoveEntity, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Actor System ApplyData"), STAT_ActorSystemApplyData, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Actor System ApplyHandover"), STAT_ActorSystemApplyHandover, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Actor System ReceiveActor"), STAT_ActorSystemReceiveActor, STATGROUP_SpatialNet);
DECLARE_CYCLE_STAT(TEXT("Actor System RemoveActor"), STAT_ActorSystemRemoveActor, STATGROUP_SpatialNet);

namespace SpatialGDK
{
struct ActorSystem::RepStateUpdateHelper
{
	RepStateUpdateHelper(USpatialActorChannel& Channel, UObject& TargetObject)
		: ObjectPtr(MakeWeakObjectPtr(&TargetObject))
		, ObjectRepState(Channel.ObjectReferenceMap.Find(ObjectPtr))
	{
	}

	~RepStateUpdateHelper() { check(bUpdatePerfomed); }

	FObjectReferencesMap& GetRefMap()
	{
		if (ObjectRepState)
		{
			return ObjectRepState->ReferenceMap;
		}
		return TempRefMap;
	}

	void Update(ActorSystem& Actors, USpatialActorChannel& Channel, const bool bReferencesChanged)
	{
		check(!bUpdatePerfomed);

		if (bReferencesChanged)
		{
			if (ObjectRepState == nullptr && TempRefMap.Num() > 0)
			{
				ObjectRepState =
					&Channel.ObjectReferenceMap.Add(ObjectPtr, FSpatialObjectRepState(FChannelObjectPair(&Channel, ObjectPtr)));
				ObjectRepState->ReferenceMap = MoveTemp(TempRefMap);
			}

			if (ObjectRepState)
			{
				ObjectRepState->UpdateRefToRepStateMap(Actors.ObjectRefToRepStateMap);

				if (ObjectRepState->ReferencedObj.Num() == 0)
				{
					Channel.ObjectReferenceMap.Remove(ObjectPtr);
				}
			}
		}
#if DO_CHECK
		bUpdatePerfomed = true;
#endif
	}

private:
	FObjectReferencesMap TempRefMap;
	TWeakObjectPtr<UObject> ObjectPtr;
	FSpatialObjectRepState* ObjectRepState;
#if DO_CHECK
	bool bUpdatePerfomed = false;
#endif
};

struct FSubViewDelta;

ActorSystem::ActorSystem(const FSubView& InSubView, const FSubView& InTombstoneSubView, USpatialNetDriver* InNetDriver,
						 FTimerManager* InTimerManager, SpatialEventTracer* InEventTracer)
	: SubView(&InSubView)
	, TombstoneSubView(&InTombstoneSubView)
	, NetDriver(InNetDriver)
	, TimerManager(InTimerManager)
	, EventTracer(InEventTracer)
{
}

void ActorSystem::Advance()
{
	for (const EntityDelta& Delta : TombstoneSubView->GetViewDelta().EntityDeltas)
	{
		if (Delta.Type == EntityDelta::ADD)
		{
			AActor* EntityActor = TryGetActor(
				UnrealMetadata(SubView->GetView()[Delta.EntityId]
								   .Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::UNREAL_METADATA_COMPONENT_ID })
								   ->GetUnderlying()));
			if (EntityActor == nullptr)
			{
				continue;
			}
			UE_LOG(LogSpatialReceiver, Verbose, TEXT("The received actor with entity ID %lld was tombstoned. The actor will be deleted."),
				   Delta.EntityId);
			// We must first Resolve the EntityId to the Actor in order for RemoveActor to succeed.
			NetDriver->PackageMap->ResolveEntityActor(EntityActor, Delta.EntityId);
			RemoveActor(Delta.EntityId);
		}
	}

	for (const EntityDelta& Delta : SubView->GetViewDelta().EntityDeltas)
	{
		switch (Delta.Type)
		{
		case EntityDelta::UPDATE:
		{
			// We process authority lost temporarily twice. Once at the start, to lose authority, and again at the end
			// to regain it. Why? Because if we temporarily lost authority, we may see surprising updates during the
			// tick, such as updates for components we would otherwise think we were authoritative over, and ignore.
			for (const AuthorityChange& Change : Delta.AuthorityLostTemporarily)
			{
				AuthorityLost(Delta.EntityId, Change.ComponentSetId);
			}
			for (const AuthorityChange& Change : Delta.AuthorityLost)
			{
				AuthorityLost(Delta.EntityId, Change.ComponentSetId);
			}
			for (const ComponentChange& Change : Delta.ComponentsAdded)
			{
				ApplyComponentAdd(Delta.EntityId, Change.ComponentId, Change.Data);
				ComponentAdded(Delta.EntityId, Change.ComponentId, Change.Data);
			}
			for (const ComponentChange& Change : Delta.ComponentUpdates)
			{
				ComponentUpdated(Delta.EntityId, Change.ComponentId, Change.Update);
			}
			for (const ComponentChange& Change : Delta.ComponentsRefreshed)
			{
				ApplyComponentAdd(Delta.EntityId, Change.ComponentId, Change.CompleteUpdate.Data);
				ComponentAdded(Delta.EntityId, Change.ComponentId, Change.CompleteUpdate.Data);
			}
			for (const ComponentChange& Change : Delta.ComponentsRemoved)
			{
				ComponentRemoved(Delta.EntityId, Change.ComponentId);
			}
			for (const AuthorityChange& Change : Delta.AuthorityGained)
			{
				AuthorityGained(Delta.EntityId, Change.ComponentSetId);
			}
			for (const AuthorityChange& Change : Delta.AuthorityLostTemporarily)
			{
				AuthorityGained(Delta.EntityId, Change.ComponentSetId);
			}
			break;
		}
		case EntityDelta::ADD:
			PopulateDataStore(Delta.EntityId);
			EntityAdded(Delta.EntityId);
			break;
		case EntityDelta::REMOVE:
			EntityRemoved(Delta.EntityId);
			ActorDataStore.Remove(Delta.EntityId);
			break;
		case EntityDelta::TEMPORARILY_REMOVED:
			EntityRemoved(Delta.EntityId);
			ActorDataStore.Remove(Delta.EntityId);
			PopulateDataStore(Delta.EntityId);
			EntityAdded(Delta.EntityId);
			break;
		default:
			break;
		}
	}
}

UnrealMetadata* ActorSystem::GetUnrealMetadata(const Worker_EntityId EntityId)
{
	if (ActorDataStore.Contains(EntityId))
	{
		return &ActorDataStore[EntityId].Metadata;
	}
	return nullptr;
}

void ActorSystem::PopulateDataStore(const Worker_EntityId EntityId)
{
	ActorData& Components = ActorDataStore.Emplace(EntityId, ActorData{});
	for (const ComponentData& Data : SubView->GetView()[EntityId].Components)
	{
		switch (Data.GetComponentId())
		{
		case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
			Components.Spawn = SpawnData(Data.GetUnderlying());
			break;
		case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
			Components.Metadata = UnrealMetadata(Data.GetUnderlying());
			break;
		default:
			break;
		}
	}
}

void ActorSystem::ApplyComponentAdd(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, Schema_ComponentData* Data)
{
	switch (ComponentId)
	{
	case SpatialConstants::SPAWN_DATA_COMPONENT_ID:
		ActorDataStore[EntityId].Spawn = SpawnData(Data);
		break;
	case SpatialConstants::UNREAL_METADATA_COMPONENT_ID:
		ActorDataStore[EntityId].Metadata = UnrealMetadata(Data);
		break;
	default:
		break;
	}
}

void ActorSystem::AuthorityLost(const Worker_EntityId EntityId, const Worker_ComponentSetId ComponentSetId)
{
	if (ComponentSetId != SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID
		&& ComponentSetId != SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID)
	{
		return;
	}

	HandleActorAuthority(EntityId, ComponentSetId, WORKER_AUTHORITY_NOT_AUTHORITATIVE);
}

void ActorSystem::AuthorityGained(Worker_EntityId EntityId, Worker_ComponentSetId ComponentSetId)
{
	if (ComponentSetId != SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID
		&& ComponentSetId != SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID)
	{
		return;
	}

	if (HasEntityBeenRequestedForDelete(EntityId))
	{
		if (ComponentSetId == SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
		{
			HandleEntityDeletedAuthority(EntityId);
		}
		return;
	}

	HandleActorAuthority(EntityId, ComponentSetId, WORKER_AUTHORITY_AUTHORITATIVE);
}

void ActorSystem::HandleActorAuthority(const Worker_EntityId EntityId, const Worker_ComponentSetId ComponentSetId,
									   const Worker_Authority Authority)
{
	AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(EntityId));
	if (Actor == nullptr)
	{
		return;
	}

	// TODO - Using bActorHadAuthority should be replaced with better tracking system to Actor entity creation [UNR-3960]
	const bool bActorHadAuthority = Actor->HasAuthority();

	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId);

	if (Channel != nullptr)
	{
		if (ComponentSetId == SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
		{
			Channel->SetServerAuthority(Authority == WORKER_AUTHORITY_AUTHORITATIVE);
		}
		else if (ComponentSetId == SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID)
		{
			Channel->SetClientAuthority(Authority == WORKER_AUTHORITY_AUTHORITATIVE);
		}
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Actor))
	{
		if (USpatialNetConnection* Connection = Cast<USpatialNetConnection>(PlayerController->GetNetConnection()))
		{
			if (Authority == WORKER_AUTHORITY_AUTHORITATIVE)
			{
				Connection->Init(EntityId);
			}
			else if (Authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE)
			{
				Connection->Disable();
			}
		}
	}

	if (NetDriver->IsServer())
	{
		// TODO UNR-955 - Remove this once batch reservation of EntityIds are in.
		if (Authority == WORKER_AUTHORITY_AUTHORITATIVE)
		{
			NetDriver->Sender->ProcessUpdatesQueuedUntilAuthority(EntityId, ComponentSetId);
		}

		// If we became authoritative over the server auth component set, set our role to be ROLE_Authority
		// and set our RemoteRole to be ROLE_AutonomousProxy if the actor has an owning connection.
		// Note: Pawn, PlayerController, and PlayerState for player-owned characters can arrive in
		// any order on non-authoritative servers, so it's possible that we don't yet know if a pawn
		// is player controlled when gaining authority over the pawn and need to wait for the player
		// state. Likewise, it's possible that the player state doesn't have a pointer to its pawn
		// yet, so we need to wait for the pawn to arrive.
		if (ComponentSetId == SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID)
		{
			if (Authority == WORKER_AUTHORITY_AUTHORITATIVE)
			{
				const bool bDormantActor = (Actor->NetDormancy >= DORM_DormantAll);

				if (IsValid(Channel) || bDormantActor)
				{
					Actor->Role = ROLE_Authority;
					Actor->RemoteRole = ROLE_SimulatedProxy;

					// bReplicates is not replicated, but this actor is replicated.
					if (!Actor->GetIsReplicated())
					{
						Actor->SetReplicates(true);
					}

					if (Actor->IsA<APlayerController>())
					{
						Actor->RemoteRole = ROLE_AutonomousProxy;
					}
					else if (APawn* Pawn = Cast<APawn>(Actor))
					{
						// The following check will return false on non-authoritative servers if the PlayerState hasn't been received yet.
						if (Pawn->IsPlayerControlled())
						{
							Pawn->RemoteRole = ROLE_AutonomousProxy;
						}
					}
					else if (const APlayerState* PlayerState = Cast<APlayerState>(Actor))
					{
						// The following check will return false on non-authoritative servers if the Pawn hasn't been received yet.
						if (APawn* PawnFromPlayerState = PlayerState->GetPawn())
						{
							if (PawnFromPlayerState->IsPlayerControlled() && PawnFromPlayerState->HasAuthority())
							{
								PawnFromPlayerState->RemoteRole = ROLE_AutonomousProxy;
							}
						}
					}

					if (!bDormantActor)
					{
						UpdateShadowData(EntityId);
					}

					// TODO - Using bActorHadAuthority should be replaced with better tracking system to Actor entity creation [UNR-3960]
					// When receiving AuthorityGained from SpatialOS, the Actor role will be ROLE_Authority iff this
					// worker is receiving entity data for the 1st time after spawning the entity. In all other cases,
					// the Actor role will have been explicitly set to ROLE_SimulatedProxy previously during the
					// entity creation flow.
					if (bActorHadAuthority)
					{
						Actor->SetActorReady(true);
					}

					// We still want to call OnAuthorityGained if the Actor migrated to this worker or was loaded from a snapshot.
					Actor->OnAuthorityGained();
				}
				else
				{
					UE_LOG(LogActorSystem, Verbose,
						   TEXT("Received authority over actor %s, with entity id %lld, which has no channel. This means it attempted to "
								"delete it earlier, when it had no authority. Retrying to delete now."),
						   *Actor->GetName(), EntityId);
					NetDriver->Sender->RetireEntity(EntityId, Actor->IsNetStartupActor());
				}
			}
			else if (Authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE)
			{
				if (Channel != nullptr)
				{
					Channel->bCreatedEntity = false;
				}

				// With load-balancing enabled, we already set ROLE_SimulatedProxy and trigger OnAuthorityLost when we
				// set AuthorityIntent to another worker. This conditional exists to dodge calling OnAuthorityLost
				// twice.
				if (Actor->Role != ROLE_SimulatedProxy)
				{
					Actor->Role = ROLE_SimulatedProxy;
					Actor->RemoteRole = ROLE_Authority;

					Actor->OnAuthorityLost();
				}
			}
		}
	}
	else if (ComponentSetId == SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID)
	{
		if (Channel != nullptr)
		{
			Channel->ClientProcessOwnershipChange(Authority == WORKER_AUTHORITY_AUTHORITATIVE);
		}

		// If we are a Pawn or PlayerController, our local role should be ROLE_AutonomousProxy. Otherwise ROLE_SimulatedProxy
		if (Actor->IsA<APawn>() || Actor->IsA<APlayerController>())
		{
			Actor->Role = (Authority == WORKER_AUTHORITY_AUTHORITATIVE) ? ROLE_AutonomousProxy : ROLE_SimulatedProxy;
		}
	}
}

void ActorSystem::ComponentAdded(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, Schema_ComponentData* Data)
{
	if (ComponentId == SpatialConstants::DORMANT_COMPONENT_ID)
	{
		HandleDormantComponentAdded(EntityId);
		return;
	}

	if (ComponentId < SpatialConstants::STARTING_GENERATED_COMPONENT_ID
		|| NetDriver->ClassInfoManager->IsGeneratedQBIMarkerComponent(ComponentId))
	{
		return;
	}

	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId);
	if (Channel == nullptr)
	{
		UE_LOG(LogActorSystem, Error,
			   TEXT("Got an add component for an entity that doesn't have an associated actor channel."
					" Entity id: %lld, component id: %d."),
			   EntityId, ComponentId);
		return;
	}
	if (Channel->bCreatedEntity)
	{
		// Allows servers to change state if they are going to be authoritative, without us overwriting it with old data.
		// TODO: UNR-3457 to remove this workaround.
		return;
	}

	HandleIndividualAddComponent(EntityId, ComponentId, Data);
}

void ActorSystem::ComponentUpdated(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId, Schema_ComponentUpdate* Update)
{
	if (ComponentId < SpatialConstants::STARTING_GENERATED_COMPONENT_ID
		|| NetDriver->ClassInfoManager->IsGeneratedQBIMarkerComponent(ComponentId))
	{
		return;
	}

	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId);
	if (Channel == nullptr)
	{
		// If there is no actor channel as a result of the actor being dormant, then assume the actor is about to become active.
		if (EntityHasComponent(EntityId, SpatialConstants::DORMANT_COMPONENT_ID))
		{
			if (AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(EntityId)))
			{
				Channel = GetOrRecreateChannelForDormantActor(Actor, EntityId);

				// As we haven't removed the dormant component just yet, this might be a single replication update where the actor
				// remains dormant. Add it back to pending dormancy so the local worker can clean up the channel. If we do process
				// a dormant component removal later in this frame, we'll clear the channel from pending dormancy channel then.
				NetDriver->AddPendingDormantChannel(Channel);
			}
			else
			{
				UE_LOG(LogActorSystem, Warning,
					   TEXT("Worker: %s Dormant actor (entity: %lld) has been deleted on this worker but we have received a component "
							"update (id: %d) from the server."),
					   *NetDriver->Connection->GetWorkerId(), EntityId, ComponentId);
				return;
			}
		}
		else
		{
			UE_LOG(LogActorSystem, Log,
				   TEXT("Worker: %s Entity: %lld Component: %d - No actor channel for update. This most likely occured due to the "
						"component updates that are sent when authority is lost during entity deletion."),
				   *NetDriver->Connection->GetWorkerId(), EntityId, ComponentId);
			return;
		}
	}

	uint32 Offset;
	bool bFoundOffset = NetDriver->ClassInfoManager->GetOffsetByComponentId(ComponentId, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogActorSystem, Warning,
			   TEXT("Worker: %s EntityId %d ComponentId %d - Could not find offset for component id when receiving a component update."),
			   *NetDriver->Connection->GetWorkerId(), EntityId, ComponentId);
		return;
	}

	UObject* TargetObject = nullptr;

	if (Offset == 0)
	{
		TargetObject = Channel->GetActor();
	}
	else
	{
		TargetObject = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(EntityId, Offset)).Get();
	}

	if (TargetObject == nullptr)
	{
		UE_LOG(LogActorSystem, Warning, TEXT("Entity: %d Component: %d - Couldn't find target object for update"), EntityId, ComponentId);
		return;
	}

	if (EventTracer != nullptr)
	{
		TArray<FSpatialGDKSpanId> CauseSpanIds = EventTracer->GetAndConsumeSpansForComponent(EntityComponentId(EntityId, ComponentId));
		EventTracer->TraceEvent(FSpatialTraceEventBuilder::CreateComponentUpdate(Channel->Actor, TargetObject, EntityId, ComponentId),
								(const Trace_SpanIdType*)CauseSpanIds.GetData(), /* NumCauses */ 1);
	}

	ESchemaComponentType Category = NetDriver->ClassInfoManager->GetCategoryByComponentId(ComponentId);

	if (Category == SCHEMA_Data || Category == SCHEMA_OwnerOnly || Category == SCHEMA_InitialOnly)
	{
		SCOPE_CYCLE_COUNTER(STAT_ActorSystemApplyData);
		ApplyComponentUpdate(ComponentId, Update, *TargetObject, *Channel, /* bIsHandover */ false);
	}
	else if (Category == SCHEMA_Handover)
	{
		SCOPE_CYCLE_COUNTER(STAT_ActorSystemApplyHandover);
		check(NetDriver->IsServer());

		ApplyComponentUpdate(ComponentId, Update, *TargetObject, *Channel, /* bIsHandover */ true);
	}
	else
	{
		UE_LOG(LogActorSystem, Verbose,
			   TEXT("Entity: %d Component: %d - Skipping because it's an empty component update from an RPC component. (most likely as a "
					"result of gaining authority)"),
			   EntityId, ComponentId);
	}
}

void ActorSystem::ComponentRemoved(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId) const
{
	// Early out if this isn't a generated component.
	if (ComponentId < SpatialConstants::STARTING_GENERATED_COMPONENT_ID && ComponentId != SpatialConstants::DORMANT_COMPONENT_ID)
	{
		return;
	}

	if (AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(EntityId).Get()))
	{
		FUnrealObjectRef ObjectRef(EntityId, ComponentId);
		if (ComponentId == SpatialConstants::DORMANT_COMPONENT_ID)
		{
			GetOrRecreateChannelForDormantActor(Actor, EntityId);
		}
		else if (UObject* Object = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(ObjectRef).Get())
		{
			if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId))
			{
				TWeakObjectPtr<UObject> WeakPtr(Object);
				Channel->OnSubobjectDeleted(ObjectRef, Object, WeakPtr);

				Actor->OnSubobjectDestroyFromReplication(Object);

				Object->PreDestroyFromReplication();
				Object->MarkPendingKill();

				NetDriver->PackageMap->RemoveSubobject(FUnrealObjectRef(EntityId, ComponentId));
			}
		}
	}
}

void ActorSystem::EntityAdded(const Worker_EntityId EntityId)
{
	ReceiveActor(EntityId);
	for (const auto& AuthoritativeComponentSet : SubView->GetView()[EntityId].Authority)
	{
		AuthorityGained(EntityId, AuthoritativeComponentSet);
	}
}

void ActorSystem::EntityRemoved(const Worker_EntityId EntityId)
{
	SCOPE_CYCLE_COUNTER(STAT_ActorSystemRemoveEntity);

	RemoveActor(EntityId);

	if (NetDriver->InitialOnlyFilter != nullptr && NetDriver->InitialOnlyFilter->HasInitialOnlyData(EntityId))
	{
		NetDriver->InitialOnlyFilter->RemoveInitialOnlyData(EntityId);
	}

	// Stop tracking if the entity was deleted as a result of deleting the actor during creation.
	// This assumes that authority will be gained before interest is gained and lost.
	const int32 RetiredActorIndex = EntitiesToRetireOnAuthorityGain.IndexOfByPredicate([EntityId](const DeferredRetire& Retire) {
		return EntityId == Retire.EntityId;
	});
	if (RetiredActorIndex != INDEX_NONE)
	{
		EntitiesToRetireOnAuthorityGain.RemoveAtSwap(RetiredActorIndex);
	}
}

bool ActorSystem::HasEntityBeenRequestedForDelete(Worker_EntityId EntityId) const
{
	return EntitiesToRetireOnAuthorityGain.ContainsByPredicate([EntityId](const DeferredRetire& Retire) {
		return EntityId == Retire.EntityId;
	});
}

void ActorSystem::HandleEntityDeletedAuthority(Worker_EntityId EntityId) const
{
	const int32 Index = EntitiesToRetireOnAuthorityGain.IndexOfByPredicate([EntityId](const DeferredRetire& Retire) {
		return Retire.EntityId == EntityId;
	});
	if (Index != INDEX_NONE)
	{
		HandleDeferredEntityDeletion(EntitiesToRetireOnAuthorityGain[Index]);
	}
}

void ActorSystem::HandleDeferredEntityDeletion(const DeferredRetire& Retire) const
{
	if (Retire.bNeedsTearOff)
	{
		NetDriver->Sender->SendActorTornOffUpdate(Retire.EntityId, Retire.ActorClassId);
		NetDriver->DelayedRetireEntity(Retire.EntityId, 1.0f, Retire.bIsNetStartupActor);
	}
	else
	{
		NetDriver->Sender->RetireEntity(Retire.EntityId, Retire.bIsNetStartupActor);
	}
}

void ActorSystem::UpdateShadowData(const Worker_EntityId EntityId) const
{
	USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId);
	ActorChannel->UpdateShadowData();
}

void ActorSystem::RetireWhenAuthoritative(Worker_EntityId EntityId, Worker_ComponentId ActorClassId, bool bIsNetStartup, bool bNeedsTearOff)
{
	DeferredRetire DeferredObj = { EntityId, ActorClassId, bIsNetStartup, bNeedsTearOff };
	EntitiesToRetireOnAuthorityGain.Add(DeferredObj);
}

void ActorSystem::HandleDormantComponentAdded(const Worker_EntityId EntityId) const
{
	if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId))
	{
		NetDriver->AddPendingDormantChannel(Channel);
	}
	else
	{
		// This would normally get registered through the channel cleanup, but we don't have one for this entity
		NetDriver->RegisterDormantEntityId(EntityId);
	}
}

void ActorSystem::HandleIndividualAddComponent(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
											   Schema_ComponentData* Data)
{
	uint32 Offset = 0;
	bool bFoundOffset = NetDriver->ClassInfoManager->GetOffsetByComponentId(ComponentId, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogActorSystem, Warning,
			   TEXT("Could not find offset for component id when receiving dynamic AddComponent."
					" (EntityId %lld, ComponentId %d)"),
			   EntityId, ComponentId);
		return;
	}

	// Object already exists, we can apply data directly.
	if (UObject* Object = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(EntityId, Offset)).Get())
	{
		if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId))
		{
			ApplyComponentData(*Channel, *Object, ComponentId, Data);
		}
		return;
	}

	const FClassInfo& Info = NetDriver->ClassInfoManager->GetClassInfoByComponentId(ComponentId);
	AActor* Actor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(EntityId).Get());
	if (Actor == nullptr)
	{
		UE_LOG(LogActorSystem, Warning,
			   TEXT("Received an add component op for subobject of type %s on entity %lld but couldn't find Actor!"),
			   *Info.Class->GetName(), EntityId);
		return;
	}

	// Check if this is a static subobject that's been destroyed by the receiver.
	if (!IsDynamicSubObject(Actor, Offset))
	{
		UE_LOG(LogActorSystem, Verbose,
			   TEXT("Tried to apply component data on add component for a static subobject that's been deleted, will skip. Entity: %lld, "
					"Component: %d, Actor: %s"),
			   EntityId, ComponentId, *Actor->GetPathName());
		return;
	}

	// Otherwise this is a dynamically attached component. We need to make sure we have all related components before creation.
	PendingDynamicSubobjectComponents.FindOrAdd(EntityId).Emplace(ComponentId);

	// We ensure that the data component is added last, so once it's in view we're ready to attach the Unreal subobject
	if (ComponentId == Info.SchemaComponents[SCHEMA_Data])
	{
		AttachDynamicSubobject(Actor, EntityId, Info);
	}
}

void ActorSystem::AttachDynamicSubobject(AActor* Actor, Worker_EntityId EntityId, const FClassInfo& Info)
{
	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId);
	if (Channel == nullptr)
	{
		UE_LOG(LogActorSystem, Verbose, TEXT("Tried to dynamically attach subobject of type %s to entity %lld but couldn't find Channel!"),
			   *Info.Class->GetName(), EntityId);
		return;
	}

	UObject* Subobject = NewObject<UObject>(Actor, Info.Class.Get());

	Actor->OnSubobjectCreatedFromReplication(Subobject);

	FUnrealObjectRef SubobjectRef(EntityId, Info.SchemaComponents[SCHEMA_Data]);
	NetDriver->PackageMap->ResolveSubobject(Subobject, SubobjectRef);

	Channel->CreateSubObjects.Add(Subobject);

	for (auto ComponentId : *PendingDynamicSubobjectComponents.Find(EntityId))
	{
		ApplyComponentData(*Channel, *Subobject, ComponentId,
						   SubView->GetView()[EntityId].Components.FindByPredicate(ComponentIdEquality{ ComponentId })->GetUnderlying());
	}
	PendingDynamicSubobjectComponents.Remove(EntityId);

	// Resolve things like RepNotify or RPCs after applying component data.
	ResolvePendingOperations(Subobject, SubobjectRef);
}

void ActorSystem::ApplyComponentData(USpatialActorChannel& Channel, UObject& TargetObject, const Worker_ComponentId ComponentId,
									 Schema_ComponentData* Data)
{
	UClass* Class = NetDriver->ClassInfoManager->GetClassByComponentId(ComponentId);
	checkf(Class, TEXT("Component %d isn't hand-written and not present in ComponentToClassMap."), ComponentId);

	ESchemaComponentType ComponentType = NetDriver->ClassInfoManager->GetCategoryByComponentId(ComponentId);

	if (ComponentType == SCHEMA_Data || ComponentType == SCHEMA_OwnerOnly || ComponentType == SCHEMA_InitialOnly)
	{
		if (ComponentType == SCHEMA_Data && TargetObject.IsA<UActorComponent>())
		{
			Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);
			bool bReplicates = !!Schema_IndexBool(ComponentObject, SpatialConstants::ACTOR_COMPONENT_REPLICATES_ID, 0);
			if (!bReplicates)
			{
				return;
			}
		}
		RepStateUpdateHelper RepStateHelper(Channel, TargetObject);

		ComponentReader Reader(NetDriver, RepStateHelper.GetRefMap(), NetDriver->Connection->GetEventTracer());
		bool bOutReferencesChanged = false;
		Reader.ApplyComponentData(ComponentId, Data, TargetObject, Channel, /* bIsHandover */ false, bOutReferencesChanged);

		RepStateHelper.Update(*this, Channel, bOutReferencesChanged);
	}
	else if (ComponentType == SCHEMA_Handover)
	{
		RepStateUpdateHelper RepStateHelper(Channel, TargetObject);

		ComponentReader Reader(NetDriver, RepStateHelper.GetRefMap(), NetDriver->Connection->GetEventTracer());
		bool bOutReferencesChanged = false;
		Reader.ApplyComponentData(ComponentId, Data, TargetObject, Channel, /* bIsHandover */ true, bOutReferencesChanged);

		RepStateHelper.Update(*this, Channel, bOutReferencesChanged);
	}
	else
	{
		UE_LOG(LogActorSystem, Verbose, TEXT("Entity: %d Component: %d - Skipping because RPC components don't have actual data."),
			   Channel.GetEntityId(), ComponentId);
	}
}

bool ActorSystem::IsDynamicSubObject(AActor* Actor, uint32 SubObjectOffset)
{
	const FClassInfo& ActorClassInfo = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());
	return !ActorClassInfo.SubobjectInfo.Contains(SubObjectOffset);
}

void ActorSystem::ResolvePendingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	UE_LOG(LogActorSystem, Verbose, TEXT("Resolving pending object refs and RPCs which depend on object: %s %s."), *Object->GetName(),
		   *ObjectRef.ToString());

	ResolveIncomingOperations(Object, ObjectRef);

	// When resolving an Actor that should uniquely exist in a deployment, e.g. GameMode, GameState, LevelScriptActors, we also
	// resolve using class path (in case any properties were set from a server that hasn't resolved the Actor yet).
	if (FUnrealObjectRef::ShouldLoadObjectFromClassPath(Object))
	{
		FUnrealObjectRef ClassObjectRef = FUnrealObjectRef::GetRefFromObjectClassPath(Object, NetDriver->PackageMap);
		if (ClassObjectRef.IsValid())
		{
			ResolveIncomingOperations(Object, ClassObjectRef);
		}
	}

	// TODO: UNR-1650 We're trying to resolve all queues, which introduces more overhead.
	NetDriver->RPCService->ProcessIncomingRPCs();
}

void ActorSystem::ResolveIncomingOperations(UObject* Object, const FUnrealObjectRef& ObjectRef)
{
	// TODO: queue up resolved objects since they were resolved during process ops
	// and then resolve all of them at the end of process ops - UNR:582

	TSet<FChannelObjectPair>* TargetObjectSet = ObjectRefToRepStateMap.Find(ObjectRef);
	if (!TargetObjectSet)
	{
		return;
	}

	UE_LOG(LogActorSystem, Verbose, TEXT("Resolving incoming operations depending on object ref %s, resolved object: %s"),
		   *ObjectRef.ToString(), *Object->GetName());

	for (auto ChannelObjectIter = TargetObjectSet->CreateIterator(); ChannelObjectIter; ++ChannelObjectIter)
	{
		USpatialActorChannel* DependentChannel = ChannelObjectIter->Key.Get();
		if (!DependentChannel)
		{
			ChannelObjectIter.RemoveCurrent();
			continue;
		}

		UObject* ReplicatingObject = ChannelObjectIter->Value.Get();

		if (!ReplicatingObject)
		{
			if (DependentChannel->ObjectReferenceMap.Find(ChannelObjectIter->Value))
			{
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				ChannelObjectIter.RemoveCurrent();
			}
			continue;
		}

		FSpatialObjectRepState* RepState = DependentChannel->ObjectReferenceMap.Find(ChannelObjectIter->Value);
		if (!RepState || !RepState->UnresolvedRefs.Contains(ObjectRef))
		{
			continue;
		}

		// Check whether the resolved object has been torn off, or is on an actor that has been torn off.
		if (AActor* AsActor = Cast<AActor>(ReplicatingObject))
		{
			if (AsActor->GetTearOff())
			{
				UE_LOG(LogSpatialActorChannel, Log,
					   TEXT("Actor to be resolved was torn off, so ignoring incoming operations. Object ref: %s, resolved object: %s"),
					   *ObjectRef.ToString(), *Object->GetName());
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				continue;
			}
		}
		else if (AActor* OuterActor = ReplicatingObject->GetTypedOuter<AActor>())
		{
			if (OuterActor->GetTearOff())
			{
				UE_LOG(LogSpatialActorChannel, Log,
					   TEXT("Owning Actor of the object to be resolved was torn off, so ignoring incoming operations. Object ref: %s, "
							"resolved object: %s"),
					   *ObjectRef.ToString(), *Object->GetName());
				DependentChannel->ObjectReferenceMap.Remove(ChannelObjectIter->Value);
				continue;
			}
		}

		bool bSomeObjectsWereMapped = false;
		TArray<GDK_PROPERTY(Property)*> RepNotifies;

		FRepLayout& RepLayout = DependentChannel->GetObjectRepLayout(ReplicatingObject);
		FRepStateStaticBuffer& ShadowData = DependentChannel->GetObjectStaticBuffer(ReplicatingObject);
		if (ShadowData.Num() == 0)
		{
			DependentChannel->ResetShadowData(RepLayout, ShadowData, ReplicatingObject);
		}

		ResolveObjectReferences(RepLayout, ReplicatingObject, *RepState, RepState->ReferenceMap, ShadowData.GetData(),
								(uint8*)ReplicatingObject, ReplicatingObject->GetClass()->GetPropertiesSize(), RepNotifies,
								bSomeObjectsWereMapped);

		if (bSomeObjectsWereMapped)
		{
			DependentChannel->RemoveRepNotifiesWithUnresolvedObjs(RepNotifies, RepLayout, RepState->ReferenceMap, ReplicatingObject);

			UE_LOG(LogActorSystem, Verbose, TEXT("Resolved for target object %s"), *ReplicatingObject->GetName());
			DependentChannel->PostReceiveSpatialUpdate(ReplicatingObject, RepNotifies, {});
		}

		RepState->UnresolvedRefs.Remove(ObjectRef);
	}
}

void ActorSystem::ResolveObjectReferences(FRepLayout& RepLayout, UObject* ReplicatedObject, FSpatialObjectRepState& RepState,
										  FObjectReferencesMap& ObjectReferencesMap, uint8* RESTRICT StoredData, uint8* RESTRICT Data,
										  int32 MaxAbsOffset, TArray<GDK_PROPERTY(Property) *>& RepNotifies,
										  bool& bOutSomeObjectsWereMapped)
{
	for (auto It = ObjectReferencesMap.CreateIterator(); It; ++It)
	{
		int32 AbsOffset = It.Key();

		if (AbsOffset >= MaxAbsOffset)
		{
			UE_LOG(LogActorSystem, Error, TEXT("ResolveObjectReferences: Removed unresolved reference: AbsOffset >= MaxAbsOffset: %d"),
				   AbsOffset);
			It.RemoveCurrent();
			continue;
		}

		FObjectReferences& ObjectReferences = It.Value();

		GDK_PROPERTY(Property)* Property = ObjectReferences.Property;

		// ParentIndex is -1 for handover properties
		bool bIsHandover = ObjectReferences.ParentIndex == -1;
		FRepParentCmd* Parent = ObjectReferences.ParentIndex >= 0 ? &RepLayout.Parents[ObjectReferences.ParentIndex] : nullptr;

		int32 StoredDataOffset = ObjectReferences.ShadowOffset;

		if (ObjectReferences.Array)
		{
			GDK_PROPERTY(ArrayProperty)* ArrayProperty = GDK_CASTFIELD<GDK_PROPERTY(ArrayProperty)>(Property);
			check(ArrayProperty != nullptr);

			if (!bIsHandover)
			{
				Property->CopySingleValue(StoredData + StoredDataOffset, Data + AbsOffset);
			}

			FScriptArray* StoredArray = bIsHandover ? nullptr : (FScriptArray*)(StoredData + StoredDataOffset);
			FScriptArray* Array = (FScriptArray*)(Data + AbsOffset);

			int32 NewMaxOffset = Array->Num() * ArrayProperty->Inner->ElementSize;

			ResolveObjectReferences(RepLayout, ReplicatedObject, RepState, *ObjectReferences.Array,
									bIsHandover ? nullptr : (uint8*)StoredArray->GetData(), (uint8*)Array->GetData(), NewMaxOffset,
									RepNotifies, bOutSomeObjectsWereMapped);
			continue;
		}

		bool bResolvedSomeRefs = false;
		UObject* SinglePropObject = nullptr;
		FUnrealObjectRef SinglePropRef = FUnrealObjectRef::NULL_OBJECT_REF;

		for (auto UnresolvedIt = ObjectReferences.UnresolvedRefs.CreateIterator(); UnresolvedIt; ++UnresolvedIt)
		{
			FUnrealObjectRef& ObjectRef = *UnresolvedIt;

			bool bUnresolved = false;
			UObject* Object = FUnrealObjectRef::ToObjectPtr(ObjectRef, NetDriver->PackageMap, bUnresolved);
			if (!bUnresolved)
			{
				check(Object != nullptr);

				UE_LOG(LogActorSystem, Verbose,
					   TEXT("ResolveObjectReferences: Resolved object ref: Offset: %d, Object ref: %s, PropName: %s, ObjName: %s"),
					   AbsOffset, *ObjectRef.ToString(), *Property->GetNameCPP(), *Object->GetName());

				if (ObjectReferences.bSingleProp)
				{
					SinglePropObject = Object;
					SinglePropRef = ObjectRef;
				}

				UnresolvedIt.RemoveCurrent();

				bResolvedSomeRefs = true;
			}
		}

		if (bResolvedSomeRefs)
		{
			if (!bOutSomeObjectsWereMapped)
			{
				ReplicatedObject->PreNetReceive();
				bOutSomeObjectsWereMapped = true;
			}

			if (Parent && Parent->Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				Property->CopySingleValue(StoredData + StoredDataOffset, Data + AbsOffset);
			}

			if (ObjectReferences.bSingleProp)
			{
				GDK_PROPERTY(ObjectPropertyBase)* ObjectProperty = GDK_CASTFIELD<GDK_PROPERTY(ObjectPropertyBase)>(Property);
				check(ObjectProperty);

				ObjectProperty->SetObjectPropertyValue(Data + AbsOffset, SinglePropObject);
				ObjectReferences.MappedRefs.Add(SinglePropRef);
			}
			else if (ObjectReferences.bFastArrayProp)
			{
				TSet<FUnrealObjectRef> NewMappedRefs;
				TSet<FUnrealObjectRef> NewUnresolvedRefs;
				FSpatialNetBitReader ValueDataReader(NetDriver->PackageMap, ObjectReferences.Buffer.GetData(),
													 ObjectReferences.NumBufferBits, NewMappedRefs, NewUnresolvedRefs);

				check(Property->IsA<GDK_PROPERTY(ArrayProperty)>());
				UScriptStruct* NetDeltaStruct = GetFastArraySerializerProperty(GDK_CASTFIELD<GDK_PROPERTY(ArrayProperty)>(Property));

				FSpatialNetDeltaSerializeInfo::DeltaSerializeRead(NetDriver, ValueDataReader, ReplicatedObject, Parent->ArrayIndex,
																  Parent->Property, NetDeltaStruct);

				ObjectReferences.MappedRefs.Append(NewMappedRefs);
			}
			else
			{
				TSet<FUnrealObjectRef> NewMappedRefs;
				TSet<FUnrealObjectRef> NewUnresolvedRefs;
				FSpatialNetBitReader BitReader(NetDriver->PackageMap, ObjectReferences.Buffer.GetData(), ObjectReferences.NumBufferBits,
											   NewMappedRefs, NewUnresolvedRefs);
				check(Property->IsA<GDK_PROPERTY(StructProperty)>());

				bool bHasUnresolved = false;
				ReadStructProperty(BitReader, GDK_CASTFIELD<GDK_PROPERTY(StructProperty)>(Property), NetDriver, Data + AbsOffset,
								   bHasUnresolved);

				ObjectReferences.MappedRefs.Append(NewMappedRefs);
			}

			if (Parent && Parent->Property->HasAnyPropertyFlags(CPF_RepNotify))
			{
				if (Parent->RepNotifyCondition == REPNOTIFY_Always || !Property->Identical(StoredData + StoredDataOffset, Data + AbsOffset))
				{
					RepNotifies.AddUnique(Parent->Property);
				}
			}
		}
	}
}

USpatialActorChannel* ActorSystem::GetOrRecreateChannelForDormantActor(AActor* Actor, const Worker_EntityId EntityID) const
{
	// Receive would normally create channel in ReceiveActor - this function is used to recreate the channel after waking up a dormant actor
	USpatialActorChannel* Channel = NetDriver->GetOrCreateSpatialActorChannel(Actor);
	if (Channel == nullptr)
	{
		return nullptr;
	}
	check(!Channel->bCreatingNewEntity);
	check(Channel->GetEntityId() == EntityID);

	NetDriver->RemovePendingDormantChannel(Channel);
	NetDriver->UnregisterDormantEntityId(EntityID);

	return Channel;
}

void ActorSystem::ApplyComponentUpdate(const Worker_ComponentId ComponentId, Schema_ComponentUpdate* ComponentUpdate, UObject& TargetObject,
									   USpatialActorChannel& Channel, bool bIsHandover)
{
	RepStateUpdateHelper RepStateHelper(Channel, TargetObject);

	ComponentReader Reader(NetDriver, RepStateHelper.GetRefMap(), NetDriver->Connection->GetEventTracer());
	bool bOutReferencesChanged = false;
	Reader.ApplyComponentUpdate(ComponentId, ComponentUpdate, TargetObject, Channel, bIsHandover, bOutReferencesChanged);
	RepStateHelper.Update(*this, Channel, bOutReferencesChanged);

	// This is a temporary workaround, see UNR-841:
	// If the update includes tearoff, close the channel and clean up the entity.
	if (TargetObject.IsA<AActor>() && NetDriver->ClassInfoManager->GetCategoryByComponentId(ComponentId) == SCHEMA_Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate);

		// Check if bTearOff has been set to true
		if (GetBoolFromSchema(ComponentObject, SpatialConstants::ACTOR_TEAROFF_ID))
		{
			Channel.ConditionalCleanUp(false, EChannelCloseReason::TearOff);
		}
	}
}

void ActorSystem::ReceiveActor(Worker_EntityId EntityId)
{
	SCOPE_CYCLE_COUNTER(STAT_ActorSystemReceiveActor);

	checkf(NetDriver, TEXT("We should have a NetDriver whilst processing ops."));
	checkf(NetDriver->GetWorld(), TEXT("We should have a World whilst processing ops."));

	ActorData& ActorComponents = ActorDataStore[EntityId];

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	AActor* EntityActor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromEntityId(EntityId));
	if (EntityActor != nullptr)
	{
		if (!EntityActor->IsActorReady())
		{
			UE_LOG(LogActorSystem, Verbose, TEXT("%s: Entity %lld for Actor %s has been checked out on the worker which spawned it."),
				   *NetDriver->Connection->GetWorkerId(), EntityId, *EntityActor->GetName());
		}

		return;
	}

	UE_LOG(LogActorSystem, Verbose,
		   TEXT("%s: Entity has been checked out on a worker which didn't spawn it. "
				"Entity ID: %lld"),
		   *NetDriver->Connection->GetWorkerId(), EntityId);

	UClass* Class = ActorComponents.Metadata.GetNativeEntityClass();
	if (Class == nullptr)
	{
		UE_LOG(LogActorSystem, Warning,
			   TEXT("The received actor with entity ID %lld couldn't be loaded. The actor (%s) will not be spawned."), EntityId,
			   *ActorComponents.Metadata.ClassPath);
		return;
	}

	// Make sure ClassInfo exists
	const FClassInfo& ActorClassInfo = NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Class);

	// If the received actor is torn off, don't bother spawning it.
	// (This is only needed due to the delay between tearoff and deleting the entity. See https://improbableio.atlassian.net/browse/UNR-841)
	if (IsReceivedEntityTornOff(EntityId))
	{
		UE_LOG(LogActorSystem, Verbose, TEXT("The received actor with entity ID %lld was already torn off. The actor will not be spawned."),
			   EntityId);
		return;
	}

	EntityActor = TryGetOrCreateActor(ActorComponents, EntityId);

	if (EntityActor == nullptr)
	{
		// This could be nullptr if:
		// a stably named actor could not be found
		// the class couldn't be loaded
		return;
	}

	UNetConnection* Connection = NetDriver->GetSpatialOSNetConnection();

	if (NetDriver->IsServer())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(EntityActor))
		{
			// If entity is a PlayerController, create channel on the PlayerController's connection.
			Connection = PlayerController->NetConnection;
		}
	}

	if (Connection == nullptr)
	{
		UE_LOG(LogActorSystem, Error,
			   TEXT("Unable to find SpatialOSNetConnection! Has this worker been disconnected from SpatialOS due to a timeout?"));
		return;
	}

	if (!NetDriver->PackageMap->ResolveEntityActor(EntityActor, EntityId))
	{
		UE_LOG(LogActorSystem, Warning,
			   TEXT("Failed to resolve entity actor when receiving entity %lld. The actor (%s) will not be spawned."), EntityId,
			   *EntityActor->GetName());
		EntityActor->Destroy(true);
		return;
	}

	// Set up actor channel.
	USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId);
	if (Channel == nullptr)
	{
		Channel = Cast<USpatialActorChannel>(Connection->CreateChannelByName(
			NAME_Actor, NetDriver->IsServer() ? EChannelCreateFlags::OpenedLocally : EChannelCreateFlags::None));
	}

	if (Channel == nullptr)
	{
		UE_LOG(LogActorSystem, Warning,
			   TEXT("Failed to create an actor channel when receiving entity %lld. The actor (%s) will not be spawned."), EntityId,
			   *EntityActor->GetName());
		EntityActor->Destroy(true);
		return;
	}

	if (Channel->Actor == nullptr)
	{
		Channel->SetChannelActor(EntityActor, ESetChannelActorFlags::None);
	}

	TArray<ObjectPtrRefPair> ObjectsToResolvePendingOpsFor;

	// Apply initial replicated properties.
	// This was moved to after FinishingSpawning because components existing only in blueprints aren't added until spawning is complete
	// Potentially we could split out the initial actor state and the initial component state
	for (const ComponentData& Component : SubView->GetView()[EntityId].Components)
	{
		if (NetDriver->ClassInfoManager->IsGeneratedQBIMarkerComponent(Component.GetComponentId())
			|| Component.GetComponentId() < SpatialConstants::STARTING_GENERATED_COMPONENT_ID)
		{
			continue;
		}
		ApplyComponentDataOnActorCreation(EntityId, Component.GetComponentId(), Component.GetUnderlying(), *Channel,
										  ObjectsToResolvePendingOpsFor);
	}

	if (NetDriver->InitialOnlyFilter != nullptr && NetDriver->InitialOnlyFilter->HasInitialOnlyData(EntityId))
	{
		for (const ComponentData& Component : NetDriver->InitialOnlyFilter->GetInitialOnlyData(EntityId))
		{
			ApplyComponentDataOnActorCreation(EntityId, Component.GetComponentId(), Component.GetUnderlying(), *Channel,
											  ObjectsToResolvePendingOpsFor);
		}
	}

	// Resolve things like RepNotify or RPCs after applying component data.
	for (const ObjectPtrRefPair& ObjectToResolve : ObjectsToResolvePendingOpsFor)
	{
		ResolvePendingOperations(ObjectToResolve.Key, ObjectToResolve.Value);
	}

	if (!NetDriver->IsServer())
	{
		// Update interest on the entity's components after receiving initial component data (so Role and RemoteRole are properly set).

		// This is a bit of a hack unfortunately, among the core classes only PlayerController implements this function and it requires
		// a player index. For now we don't support split screen, so the number is always 0.
		if (EntityActor->IsA(APlayerController::StaticClass()))
		{
			uint8 PlayerIndex = 0;
			// FInBunch takes size in bits not bytes
			FInBunch Bunch(NetDriver->ServerConnection, &PlayerIndex, sizeof(PlayerIndex) * 8);
			EntityActor->OnActorChannelOpen(Bunch, NetDriver->ServerConnection);
		}
		else
		{
			FInBunch Bunch(NetDriver->ServerConnection);
			EntityActor->OnActorChannelOpen(Bunch, NetDriver->ServerConnection);
		}
	}

	// Any Actor created here will have been received over the wire as an entity so we can mark it ready.
	EntityActor->SetActorReady(NetDriver->IsServer() && EntityActor->bNetStartup);

	// Taken from PostNetInit
	if (NetDriver->GetWorld()->HasBegunPlay() && !EntityActor->HasActorBegunPlay())
	{
		EntityActor->DispatchBeginPlay();
	}

	EntityActor->UpdateOverlaps();

	if (EntityHasComponent(EntityId, SpatialConstants::DORMANT_COMPONENT_ID))
	{
		NetDriver->AddPendingDormantChannel(Channel);
	}
}

bool ActorSystem::IsReceivedEntityTornOff(const Worker_EntityId EntityId) const
{
	// Check the pending add components, to find the root component for the received entity.
	for (const ComponentData& Data : SubView->GetView()[EntityId].Components)
	{
		if (NetDriver->ClassInfoManager->GetCategoryByComponentId(Data.GetComponentId()) != SCHEMA_Data)
		{
			continue;
		}

		UClass* Class = NetDriver->ClassInfoManager->GetClassByComponentId(Data.GetComponentId());
		if (!Class->IsChildOf<AActor>())
		{
			continue;
		}

		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.GetUnderlying());
		return GetBoolFromSchema(ComponentObject, SpatialConstants::ACTOR_TEAROFF_ID);
	}

	return false;
}

AActor* ActorSystem::TryGetActor(const UnrealMetadata& Metadata) const
{
	if (Metadata.StablyNamedRef.IsSet())
	{
		if (NetDriver->IsServer() || Metadata.bNetStartup.GetValue())
		{
			// This Actor already exists in the map, get it from the package map.
			const FUnrealObjectRef& StablyNamedRef = Metadata.StablyNamedRef.GetValue();
			AActor* StaticActor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromUnrealObjectRef(StablyNamedRef));
			// An unintended side effect of GetObjectFromUnrealObjectRef is that this ref
			// will be registered with this Actor. It can be the case that this Actor is not
			// stably named (due to bNetLoadOnClient = false) so we should let
			// SpatialPackageMapClient::ResolveEntityActor handle it properly.
			NetDriver->PackageMap->UnregisterActorObjectRefOnly(StablyNamedRef);

			return StaticActor;
		}
	}
	return nullptr;
}

AActor* ActorSystem::TryGetOrCreateActor(ActorData& ActorComponents, const Worker_EntityId EntityId)
{
	if (ActorComponents.Metadata.StablyNamedRef.IsSet())
	{
		if (NetDriver->IsServer() || ActorComponents.Metadata.bNetStartup.GetValue())
		{
			// This Actor already exists in the map, get it from the package map.
			const FUnrealObjectRef& StablyNamedRef = ActorComponents.Metadata.StablyNamedRef.GetValue();
			AActor* StaticActor = Cast<AActor>(NetDriver->PackageMap->GetObjectFromUnrealObjectRef(StablyNamedRef));
			// An unintended side effect of GetObjectFromUnrealObjectRef is that this ref
			// will be registered with this Actor. It can be the case that this Actor is not
			// stably named (due to bNetLoadOnClient = false) so we should let
			// SpatialPackageMapClient::ResolveEntityActor handle it properly.
			NetDriver->PackageMap->UnregisterActorObjectRefOnly(StablyNamedRef);

			return StaticActor;
		}
	}

	// Handle linking received unique Actors (e.g. game state, game mode) to instances already spawned on this worker.
	UClass* ActorClass = ActorComponents.Metadata.GetNativeEntityClass();
	if (FUnrealObjectRef::IsUniqueActorClass(ActorClass) && NetDriver->IsServer())
	{
		return NetDriver->PackageMap->GetUniqueActorInstanceByClass(ActorClass);
	}

	return CreateActor(ActorComponents, EntityId);
}

// This function is only called for client and server workers who did not spawn the Actor
AActor* ActorSystem::CreateActor(ActorData& ActorComponents, const Worker_EntityId EntityId)
{
	UClass* ActorClass = ActorComponents.Metadata.GetNativeEntityClass();

	if (ActorClass == nullptr)
	{
		UE_LOG(LogActorSystem, Error, TEXT("Could not load class %s when spawning entity!"), *ActorComponents.Metadata.ClassPath);
		return nullptr;
	}

	UE_LOG(LogActorSystem, Verbose, TEXT("Spawning a %s whilst checking out an entity."), *ActorClass->GetFullName());

	const bool bCreatingPlayerController = ActorClass->IsChildOf(APlayerController::StaticClass());

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.bRemoteOwned = true;
	SpawnInfo.bNoFail = true;

	FVector SpawnLocation = FRepMovement::RebaseOntoLocalOrigin(ActorComponents.Spawn.Location, NetDriver->GetWorld()->OriginLocation);

	AActor* NewActor =
		NetDriver->GetWorld()->SpawnActorAbsolute(ActorClass, FTransform(ActorComponents.Spawn.Rotation, SpawnLocation), SpawnInfo);
	check(NewActor);

	if (NetDriver->IsServer() && bCreatingPlayerController)
	{
		// Grab the client system entity ID from the partition component in order to correctly link this
		// connection to the client it corresponds to.
		const Worker_EntityId ClientSystemEntityId =
			Partition(SubView->GetView()[EntityId]
						  .Components.FindByPredicate(ComponentIdEquality{ SpatialConstants::PARTITION_COMPONENT_ID })
						  ->GetUnderlying())
				.WorkerConnectionId;

		NetDriver->PostSpawnPlayerController(Cast<APlayerController>(NewActor), ClientSystemEntityId);
	}

	// Imitate the behavior in UPackageMapClient::SerializeNewActor.
	const float Epsilon = 0.001f;
	if (ActorComponents.Spawn.Velocity.Equals(FVector::ZeroVector, Epsilon))
	{
		NewActor->PostNetReceiveVelocity(ActorComponents.Spawn.Velocity);
	}
	if (!ActorComponents.Spawn.Scale.Equals(FVector::OneVector, Epsilon))
	{
		NewActor->SetActorScale3D(ActorComponents.Spawn.Scale);
	}

	// Don't have authority over Actor until SpatialOS delegates authority
	NewActor->Role = ROLE_SimulatedProxy;
	NewActor->RemoteRole = ROLE_Authority;

	return NewActor;
}

void ActorSystem::ApplyComponentDataOnActorCreation(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId,
													Schema_ComponentData* Data, USpatialActorChannel& Channel,
													TArray<ObjectPtrRefPair>& OutObjectsToResolve)
{
	AActor* Actor = Channel.GetActor();

	uint32 Offset = 0;
	const bool bFoundOffset = NetDriver->ClassInfoManager->GetOffsetByComponentId(ComponentId, Offset);
	if (!bFoundOffset)
	{
		UE_LOG(LogActorSystem, Warning,
			   TEXT("Worker: %s EntityId: %lld, ComponentId: %d - Could not find offset for component id when applying component data to "
					"Actor %s!"),
			   *NetDriver->Connection->GetWorkerId(), EntityId, ComponentId, *Actor->GetPathName());
		return;
	}

	FUnrealObjectRef TargetObjectRef(EntityId, Offset);
	TWeakObjectPtr<UObject> TargetObject = NetDriver->PackageMap->GetObjectFromUnrealObjectRef(TargetObjectRef);
	if (!TargetObject.IsValid())
	{
		if (!IsDynamicSubObject(Actor, Offset))
		{
			UE_LOG(LogActorSystem, Verbose,
				   TEXT("Tried to apply component data on actor creation for a static subobject that's been deleted, will skip. Entity: "
						"%lld, Component: %d, Actor: %s"),
				   EntityId, ComponentId, *Actor->GetPathName());
			return;
		}

		// If we can't find this subobject, it's a dynamically attached object. Check if we created previously.
		if (FNetworkGUID* SubobjectNetGUID = NetDriver->PackageMap->GetRemovedDynamicSubobjectNetGUID(TargetObjectRef))
		{
			if (UObject* DynamicSubobject = NetDriver->PackageMap->GetObjectFromNetGUID(*SubobjectNetGUID, false))
			{
				NetDriver->PackageMap->ResolveSubobject(DynamicSubobject, TargetObjectRef);
				ApplyComponentData(Channel, *DynamicSubobject, ComponentId, Data);

				OutObjectsToResolve.Add(ObjectPtrRefPair(DynamicSubobject, TargetObjectRef));
				return;
			}
		}

		// If the dynamically attached object was not created before. Create it now.
		TargetObject = NewObject<UObject>(Actor, NetDriver->ClassInfoManager->GetClassByComponentId(ComponentId));

		Actor->OnSubobjectCreatedFromReplication(TargetObject.Get());

		NetDriver->PackageMap->ResolveSubobject(TargetObject.Get(), TargetObjectRef);

		Channel.CreateSubObjects.Add(TargetObject.Get());
	}

	FString TargetObjectPath = TargetObject->GetPathName();
	ApplyComponentData(Channel, *TargetObject, ComponentId, Data);

	if (TargetObject.IsValid())
	{
		OutObjectsToResolve.Add(ObjectPtrRefPair(TargetObject.Get(), TargetObjectRef));
	}
	else
	{
		// TODO: remove / downgrade this to a log after verifying we handle this properly - UNR-4379
		UE_LOG(LogActorSystem, Warning, TEXT("Actor subobject got invalidated after applying component data! Subobject: %s"),
			   *TargetObjectPath);
	}
}

void ActorSystem::RemoveActor(const Worker_EntityId EntityId)
{
	SCOPE_CYCLE_COUNTER(STAT_ActorSystemRemoveActor);

	TWeakObjectPtr<UObject> WeakActor = NetDriver->PackageMap->GetObjectFromEntityId(EntityId);

	// Actor has not been resolved yet or has already been destroyed. Clean up surrounding bookkeeping.
	if (!WeakActor.IsValid())
	{
		DestroyActor(nullptr, EntityId);
		return;
	}

	AActor* Actor = Cast<AActor>(WeakActor.Get());

	UE_LOG(LogActorSystem, Verbose, TEXT("Worker %s Remove Actor: %s %lld"), *NetDriver->Connection->GetWorkerId(),
		   Actor && !Actor->IsPendingKill() ? *Actor->GetName() : TEXT("nullptr"), EntityId);

	// Cleanup pending add components if any exist.
	if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
	{
		// If we have any pending subobjects on the channel, remove them
		if (ActorChannel->PendingDynamicSubobjects.Num() > 0)
		{
			PendingDynamicSubobjectComponents.Remove(EntityId);
		}
	}

	// Actor already deleted (this worker was most likely authoritative over it and deleted it earlier).
	if (Actor == nullptr || Actor->IsPendingKill())
	{
		if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
		{
			UE_LOG(LogActorSystem, Warning,
				   TEXT("RemoveActor: actor for entity %lld was already deleted (likely on the authoritative worker) but still has an open "
						"actor channel."),
				   EntityId);
			ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::Destroyed);
		}
		return;
	}

	if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
	{
		if (NetDriver->GetWorld() != nullptr && !NetDriver->GetWorld()->IsPendingKillOrUnreachable())
		{
			for (UObject* SubObject : ActorChannel->CreateSubObjects)
			{
				if (SubObject)
				{
					FUnrealObjectRef ObjectRef =
						FUnrealObjectRef::FromObjectPtr(SubObject, Cast<USpatialPackageMapClient>(NetDriver->PackageMap));
					// Unmap this object so we can remap it if it becomes relevant again in the future
					MoveMappedObjectToUnmapped(ObjectRef);
				}
			}

			FUnrealObjectRef ObjectRef = FUnrealObjectRef::FromObjectPtr(Actor, Cast<USpatialPackageMapClient>(NetDriver->PackageMap));
			// Unmap this object so we can remap it if it becomes relevant again in the future
			MoveMappedObjectToUnmapped(ObjectRef);
		}

		for (auto& ChannelRefs : ActorChannel->ObjectReferenceMap)
		{
			CleanupRepStateMap(ChannelRefs.Value);
		}

		ActorChannel->ObjectReferenceMap.Empty();

		// If the entity is to be deleted after having been torn off, ignore the request (but clean up the channel if it has not been
		// cleaned up already).
		if (Actor->GetTearOff())
		{
			ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::TearOff);
			return;
		}
	}

	// Actor is a startup actor that is a part of the level. If it's not Tombstone'd, then it
	// has just fallen out of our view and we should only remove the entity.
	if (Actor->IsFullNameStableForNetworking() && !SubView->HasComponent(EntityId, SpatialConstants::TOMBSTONE_COMPONENT_ID))
	{
		NetDriver->PackageMap->ClearRemovedDynamicSubobjectObjectRefs(EntityId);
		if (USpatialActorChannel* Channel = NetDriver->GetActorChannelByEntityId(EntityId))
		{
			for (UObject* DynamicSubobject : Channel->CreateSubObjects)
			{
				FNetworkGUID SubobjectNetGUID = NetDriver->PackageMap->GetNetGUIDFromObject(DynamicSubobject);
				if (SubobjectNetGUID.IsValid())
				{
					FUnrealObjectRef SubobjectRef = NetDriver->PackageMap->GetUnrealObjectRefFromNetGUID(SubobjectNetGUID);

					if (SubobjectRef.IsValid() && IsDynamicSubObject(Actor, SubobjectRef.Offset))
					{
						NetDriver->PackageMap->AddRemovedDynamicSubobjectObjectRef(SubobjectRef, SubobjectNetGUID);
					}
				}
			}
		}
		// We can't call CleanupDeletedEntity here as we need the NetDriver to maintain the EntityId
		// to Actor Channel mapping for the DestroyActor to function correctly
		NetDriver->PackageMap->RemoveEntityActor(EntityId);
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(Actor))
	{
		// Force APlayerController::DestroyNetworkActorHandled to return false
		PC->Player = nullptr;
	}

	// Workaround for camera loss on handover: prevent UnPossess() (non-authoritative destruction of pawn, while being authoritative over
	// the controller)
	// TODO: Check how AI controllers are affected by this (UNR-430)
	// TODO: This should be solved properly by working sets (UNR-411)
	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		AController* Controller = Pawn->Controller;

		if (Controller && Controller->HasAuthority())
		{
			Pawn->Controller = nullptr;
		}
	}

	DestroyActor(Actor, EntityId);
}

void ActorSystem::DestroyActor(AActor* Actor, const Worker_EntityId EntityId)
{
	// Destruction of actors can cause the destruction of associated actors (eg. Character > Controller). Actor destroy
	// calls will eventually find their way into USpatialActorChannel::DeleteEntityIfAuthoritative() which checks if the entity
	// is currently owned by this worker before issuing an entity delete request. If the associated entity is still authoritative
	// on this server, we need to make sure this worker doesn't issue an entity delete request, as this entity is really
	// transitioning to the same server as the actor we're currently operating on, and is just a few frames behind.
	// We make the assumption that if we're destroying actors here (due to a remove entity op), then this is only due to two
	// situations;
	// 1. Actor's entity has been transitioned to another server
	// 2. The Actor was deleted on another server
	// In neither situation do we want to delete associated entities, so prevent them from being issued.
	// TODO: fix this with working sets (UNR-411)
	NetDriver->StartIgnoringAuthoritativeDestruction();

	// Clean up the actor channel. For clients, this will also call destroy on the actor.
	if (USpatialActorChannel* ActorChannel = NetDriver->GetActorChannelByEntityId(EntityId))
	{
		ActorChannel->ConditionalCleanUp(false, EChannelCloseReason::Destroyed);
	}
	else
	{
		if (NetDriver->IsDormantEntity(EntityId))
		{
			NetDriver->PackageMap->RemoveEntityActor(EntityId);
		}
		else
		{
			UE_LOG(LogActorSystem, Verbose,
				   TEXT("Removing actor as a result of a remove entity op, which has a missing actor channel. Actor: %s EntityId: %lld"),
				   *GetNameSafe(Actor), EntityId);
		}
	}

	if (APlayerController* PC = Cast<APlayerController>(Actor))
	{
		NetDriver->CleanUpServerConnectionForPC(PC);
	}

	// It is safe to call AActor::Destroy even if the destruction has already started.
	if (Actor != nullptr && !Actor->Destroy(true))
	{
		UE_LOG(LogActorSystem, Error, TEXT("Failed to destroy actor in RemoveActor %s %lld"), *Actor->GetName(), EntityId);
	}
	NetDriver->StopIgnoringAuthoritativeDestruction();

	check(NetDriver->PackageMap->GetObjectFromEntityId(EntityId) == nullptr);
}

void ActorSystem::MoveMappedObjectToUnmapped(const FUnrealObjectRef& Ref)
{
	if (TSet<FChannelObjectPair>* RepStatesWithMappedRef = ObjectRefToRepStateMap.Find(Ref))
	{
		for (const FChannelObjectPair& ChannelObject : *RepStatesWithMappedRef)
		{
			if (USpatialActorChannel* Channel = ChannelObject.Key.Get())
			{
				if (FSpatialObjectRepState* RepState = Channel->ObjectReferenceMap.Find(ChannelObject.Value))
				{
					RepState->MoveMappedObjectToUnmapped(Ref);
				}
			}
		}
	}
}

void ActorSystem::CleanupRepStateMap(FSpatialObjectRepState& RepState)
{
	for (const FUnrealObjectRef& Ref : RepState.ReferencedObj)
	{
		TSet<FChannelObjectPair>* RepStatesWithMappedRef = ObjectRefToRepStateMap.Find(Ref);
		if (ensureMsgf(RepStatesWithMappedRef,
					   TEXT("Ref to entity %lld on object %s is missing its referenced entry in the Ref/RepState map"), Ref.Entity,
					   *GetObjectNameFromRepState(RepState)))
		{
			checkf(RepStatesWithMappedRef->Contains(RepState.GetChannelObjectPair()),
				   TEXT("Ref to entity %lld on object %s is missing its referenced entry in the Ref/RepState map"), Ref.Entity,
				   *GetObjectNameFromRepState(RepState));
			RepStatesWithMappedRef->Remove(RepState.GetChannelObjectPair());
			if (RepStatesWithMappedRef->Num() == 0)
			{
				ObjectRefToRepStateMap.Remove(Ref);
			}
		}
	}
}

FString ActorSystem::GetObjectNameFromRepState(const FSpatialObjectRepState& RepState)
{
	if (UObject* Obj = RepState.GetChannelObjectPair().Value.Get())
	{
		return Obj->GetName();
	}
	return TEXT("<unknown>");
}

bool ActorSystem::EntityHasComponent(const Worker_EntityId EntityId, const Worker_ComponentId ComponentId) const
{
	if (const auto Entity = SubView->GetView().Find(EntityId))
	{
		return Entity->Components.ContainsByPredicate(ComponentIdEquality{ ComponentId });
	}
	return false;
}

} // namespace SpatialGDK
