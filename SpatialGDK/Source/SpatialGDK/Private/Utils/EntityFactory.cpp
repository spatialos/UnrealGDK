// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/EntityFactory.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/RPCs/SpatialRPCService.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/ClientRPCEndpointLegacy.h"
#include "Schema/ComponentPresence.h"
#include "Schema/Heartbeat.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/RPCPayload.h"
#include "Schema/ServerRPCEndpointLegacy.h"
#include "Schema/SpatialDebugging.h"
#include "Schema/SpawnData.h"
#include "Schema/StandardLibrary.h"
#include "Schema/Tombstone.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "Utils/ComponentFactory.h"
#include "Utils/InspectionColors.h"
#include "Utils/InterestFactory.h"
#include "Utils/SpatialActorUtils.h"
#include "Utils/SpatialDebugger.h"

#include "Engine/Engine.h"
#include "Engine/LevelScriptActor.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"

DEFINE_LOG_CATEGORY(LogEntityFactory);

namespace SpatialGDK
{
EntityFactory::EntityFactory(USpatialNetDriver* InNetDriver, USpatialPackageMapClient* InPackageMap,
							 USpatialClassInfoManager* InClassInfoManager, SpatialRPCService* InRPCService)
	: NetDriver(InNetDriver)
	, PackageMap(InPackageMap)
	, ClassInfoManager(InClassInfoManager)
	, RPCService(InRPCService)
{
}

EntityComponents EntityFactory::CreateSkeletonEntityComponents(AActor* Actor)
{
	UClass* Class = Actor->GetClass();

	EntityComponents EntityComps;
	EntityComps.MutableComponents.Add(SpatialConstants::POSITION_COMPONENT_ID,
									  MakeUnique<Position>(Coordinates::FromFVector(GetActorSpatialPosition(Actor))));
	EntityComps.MutableComponents.Add(SpatialConstants::METADATA_COMPONENT_ID, MakeUnique<Metadata>(Class->GetName()));
	EntityComps.MutableComponents.Add(SpatialConstants::SPAWN_DATA_COMPONENT_ID, MakeUnique<SpawnData>(Actor));

	if (ShouldActorHaveVisibleComponent(Actor))
	{
		EntityComps.ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::VISIBLE_COMPONENT_ID));
	}

	if (!Class->HasAnySpatialClassFlags(SPATIALCLASS_NotPersistent))
	{
		EntityComps.ComponentDatas.Add(Persistence().CreateComponentData());
	}

	if (Actor->NetDormancy >= DORM_DormantAll)
	{
		EntityComps.ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::DORMANT_COMPONENT_ID));
	}

	if (Actor->IsA<APlayerController>())
	{
		EntityComps.ComponentDatas.Add(Heartbeat().CreateComponentData());
	}

	EntityComps.MutableComponents.Add(SpatialConstants::AUTHORITY_DELEGATION_COMPONENT_ID, MakeUnique<AuthorityDelegation>());
	EntityComps.ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID));

	// Add Actor completeness tags.
	EntityComps.ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID));
	EntityComps.ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ACTOR_NON_AUTH_TAG_COMPONENT_ID));
	EntityComps.ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::LB_TAG_COMPONENT_ID));

	return EntityComps;
}

void EntityFactory::WriteLBComponents(EntityComponents& EntityComps, AActor* Actor)
{
	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());

	const Worker_EntityId AuthoritativeClientPartitionId = GetConnectionOwningPartitionId(Actor);
	const Worker_EntityId AuthoritativeServerPartitionId = NetDriver->VirtualWorkerTranslator->GetClaimedPartitionId();

	// Add Load Balancer Attribute. If this is a single worker deployment, this will be just be the single worker.
	const VirtualWorkerId IntendedVirtualWorkerId = NetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId();
	checkf(IntendedVirtualWorkerId != SpatialConstants::INVALID_VIRTUAL_WORKER_ID,
		   TEXT("Load balancing strategy provided invalid local virtual worker ID during Actor spawn. "
				"Actor: %s. Strategy: %s"),
		   *Actor->GetName(), *NetDriver->LoadBalanceStrategy->GetName());

	AbstractMutableComponent* AuthorityDelegationComponent = EntityComps.MutableComponents[AuthorityDelegation::ComponentId].Get();
	AuthorityDelegationMap& DelegationMap = (static_cast<AuthorityDelegation*>(AuthorityDelegationComponent))->Delegations;
	DelegationMap.Add(SpatialConstants::WELL_KNOWN_COMPONENT_SET_ID, AuthoritativeServerPartitionId);
	DelegationMap.Add(SpatialConstants::SPAWN_DATA_COMPONENT_ID, AuthoritativeServerPartitionId);
	DelegationMap.Add(SpatialConstants::DORMANT_COMPONENT_ID, AuthoritativeServerPartitionId);
	DelegationMap.Add(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID, AuthoritativeServerPartitionId);
	DelegationMap.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, AuthoritativeServerPartitionId);
	DelegationMap.Add(SpatialConstants::UNREAL_METADATA_COMPONENT_ID, AuthoritativeServerPartitionId);
	DelegationMap.Add(SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID, AuthoritativeServerPartitionId);
	DelegationMap.Add(SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID, AuthoritativeServerPartitionId);
	DelegationMap.Add(SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID, AuthoritativeServerPartitionId);

	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();
	if (SpatialSettings->UseRPCRingBuffer() && RPCService != nullptr)
	{
		DelegationMap.Add(SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, AuthoritativeClientPartitionId);
		DelegationMap.Add(SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID, AuthoritativeServerPartitionId);
		DelegationMap.Add(SpatialConstants::MULTICAST_RPCS_COMPONENT_ID, AuthoritativeServerPartitionId);
	}
	else
	{
		DelegationMap.Add(SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY, AuthoritativeServerPartitionId);
		DelegationMap.Add(SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_LEGACY, AuthoritativeServerPartitionId);
		DelegationMap.Add(SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY, AuthoritativeClientPartitionId);
	}

	if (Actor->IsNetStartupActor())
	{
		DelegationMap.Add(SpatialConstants::TOMBSTONE_COMPONENT_ID, AuthoritativeServerPartitionId);
	}

	// If Actor is a PlayerController, add the heartbeat component.
	if (Actor->IsA<APlayerController>())
	{
#if !UE_BUILD_SHIPPING
		DelegationMap.Add(SpatialConstants::DEBUG_METRICS_COMPONENT_ID, AuthoritativeServerPartitionId);
#endif // !UE_BUILD_SHIPPING
		DelegationMap.Add(SpatialConstants::HEARTBEAT_COMPONENT_ID, AuthoritativeClientPartitionId);
	}

	DelegationMap.Add(SpatialConstants::VISIBLE_COMPONENT_ID, AuthoritativeServerPartitionId);

	// Add all Interest component IDs to allow us to change it if needed.
	DelegationMap.Add(SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID, AuthoritativeServerPartitionId);
	DelegationMap.Add(SpatialConstants::SERVER_ONLY_ALWAYS_RELEVANT_COMPONENT_ID, AuthoritativeServerPartitionId);
	for (const auto ComponentId : ClassInfoManager->SchemaDatabase->NetCullDistanceComponentIds)
	{
		DelegationMap.Add(ComponentId, AuthoritativeServerPartitionId);
	}

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type) {
		Worker_ComponentId ComponentId = Info.SchemaComponents[Type];
		if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		DelegationMap.Add(ComponentId, AuthoritativeServerPartitionId);
	});

	// Add debugging utilities, if we are not compiling a shipping build
#if !UE_BUILD_SHIPPING
	DelegationMap.Add(SpatialConstants::GDK_DEBUG_COMPONENT_ID, AuthoritativeServerPartitionId);
	if (NetDriver->SpatialDebugger != nullptr)
	{
		check(NetDriver->VirtualWorkerTranslator != nullptr);

		const PhysicalWorkerName* PhysicalWorkerName =
			NetDriver->VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(IntendedVirtualWorkerId);
		FColor InvalidServerTintColor = NetDriver->SpatialDebugger->InvalidServerTintColor;
		FColor IntentColor =
			PhysicalWorkerName != nullptr ? SpatialGDK::GetColorForWorkerName(*PhysicalWorkerName) : InvalidServerTintColor;

		const bool bIsLocked = NetDriver->LockingPolicy->IsLocked(Actor);

		SpatialDebugging DebuggingInfo(SpatialConstants::INVALID_VIRTUAL_WORKER_ID, InvalidServerTintColor, IntendedVirtualWorkerId,
									   IntentColor, bIsLocked);
		EntityComps.ComponentDatas.Add(DebuggingInfo.CreateComponentData());
		DelegationMap.Add(SpatialConstants::SPATIAL_DEBUGGING_COMPONENT_ID, AuthoritativeServerPartitionId);
	}
#endif // !UE_BUILD_SHIPPING

	for (const auto& Pair : EntityComps.ComponentsToDelegateToAuthoritativeWorker)
	{
		DelegationMap.Add(Pair.Key, AuthoritativeServerPartitionId);
	}

	// Add actual load balancing components
	EntityComps.ComponentDatas.Add(NetOwningClientWorker(AuthoritativeClientPartitionId).CreateComponentData());
	EntityComps.ComponentDatas.Add(AuthorityIntent::CreateAuthorityIntentData(IntendedVirtualWorkerId));
}

void EntityFactory::WriteUnrealComponents(EntityComponents& EntityComps, USpatialActorChannel* Channel,
										  FRPCsOnEntityCreationMap& OutgoingOnCreateEntityRPCs, uint32& OutBytesWritten)
{
	AActor* Actor = Channel->Actor;
	UClass* Class = Actor->GetClass();
	Worker_EntityId EntityId = Channel->GetEntityId();

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Class);

	Worker_ComponentId ActorInterestComponentId = ClassInfoManager->ComputeActorInterestComponentId(Actor);
	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();

	// We want to have a stably named ref if this is an Actor placed in the world.
	// We use this to indicate if a new Actor should be created, or to link a pre-existing Actor when receiving an AddEntityOp.
	// Previously, IsFullNameStableForNetworking was used but this was only true if bNetLoadOnClient=true.
	// Actors with bNetLoadOnClient=false also need a StablyNamedObjectRef for linking in the case of loading from a snapshot or the server
	// crashes and restarts.
	TSchemaOption<FUnrealObjectRef> StablyNamedObjectRef;
	TSchemaOption<bool> bNetStartup;
	if (Actor->HasAnyFlags(RF_WasLoaded) || Actor->bNetStartup)
	{
		// Since we've already received the EntityId for this Actor. It is guaranteed to be resolved
		// with the package map by this point
		FUnrealObjectRef OuterObjectRef = PackageMap->GetUnrealObjectRefFromObject(Actor->GetOuter());
		if (OuterObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
		{
			FNetworkGUID NetGUID = PackageMap->ResolveStablyNamedObject(Actor->GetOuter());
			OuterObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
		}

		// No path in SpatialOS should contain a PIE prefix.
		FString TempPath = Actor->GetFName().ToString();
		GEngine->NetworkRemapPath(NetDriver, TempPath, false /*bIsReading*/);

		StablyNamedObjectRef = FUnrealObjectRef(0, 0, TempPath, OuterObjectRef, true);
		bNetStartup = Actor->bNetStartup;
	}

	TArray<FWorkerComponentData>& ComponentDatas = EntityComps.ComponentDatas;
	ComponentDatas.Add(UnrealMetadata(StablyNamedObjectRef, Class->GetPathName(), bNetStartup).CreateComponentData());

	if (ActorInterestComponentId != SpatialConstants::INVALID_COMPONENT_ID)
	{
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(ActorInterestComponentId));
	}

#if !UE_BUILD_SHIPPING
	if (Actor->IsA<APlayerController>())
	{
		EntityComps.ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::DEBUG_METRICS_COMPONENT_ID));
	}
#endif // !UE_BUILD_SHIPPING

	USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(Actor);

	ComponentFactory DataFactory(false, NetDriver, Tracer);

	FRepChangeState InitialRepChanges = Channel->CreateInitialRepChangeState(Actor);
	FHandoverChangeState InitialHandoverChanges = Channel->CreateInitialHandoverChangeState(Info);

	TArray<FWorkerComponentData> DynamicComponentDatas =
		DataFactory.CreateComponentDatas(Actor, Info, InitialRepChanges, InitialHandoverChanges, OutBytesWritten);

	ComponentDatas.Append(DynamicComponentDatas);

	ComponentDatas.Add(NetDriver->InterestFactory->CreateInterestData(Actor, Info, EntityId));

	Channel->SetNeedOwnerInterestUpdate(!NetDriver->InterestFactory->DoOwnersHaveEntityId(Actor));

	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID));

	if (SpatialSettings->UseRPCRingBuffer() && RPCService != nullptr)
	{
		ComponentDatas.Append(RPCService->GetRPCComponentsOnEntityCreation(EntityId));
	}
	else
	{
		ComponentDatas.Add(ClientRPCEndpointLegacy().CreateComponentData());
		ComponentDatas.Add(ServerRPCEndpointLegacy().CreateComponentData());
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_LEGACY));

		if (RPCsOnEntityCreation* QueuedRPCs = OutgoingOnCreateEntityRPCs.Find(Actor))
		{
			if (QueuedRPCs->HasRPCPayloadData())
			{
				EntityComps.ComponentsToDelegateToAuthoritativeWorker.Add(SpatialConstants::RPCS_ON_ENTITY_CREATION_ID,
																		  QueuedRPCs->CreateComponentData());
			}
			OutgoingOnCreateEntityRPCs.Remove(Actor);
		}
	}

	// Only add subobjects which are replicating
	for (auto RepSubobject = Channel->ReplicationMap.CreateIterator(); RepSubobject; ++RepSubobject)
	{
		if (UObject* Subobject = RepSubobject.Value()->GetWeakObjectPtr().Get())
		{
			if (Subobject == Actor)
			{
				// Actor's replicator is also contained in ReplicationMap.
				continue;
			}

			// If this object is not in the PackageMap, it has been dynamically created.
			if (!PackageMap->GetUnrealObjectRefFromObject(Subobject).IsValid())
			{
				const FClassInfo* SubobjectInfo = PackageMap->TryResolveNewDynamicSubobjectAndGetClassInfo(Subobject);

				if (SubobjectInfo == nullptr)
				{
					// This is a failure but there is already a log inside TryResolveNewDynamicSubbojectAndGetClassInfo
					continue;
				}
			}

			const FClassInfo& SubobjectInfo = ClassInfoManager->GetOrCreateClassInfoByObject(Subobject);

			FRepChangeState SubobjectRepChanges = Channel->CreateInitialRepChangeState(Subobject);
			FHandoverChangeState SubobjectHandoverChanges = Channel->CreateInitialHandoverChangeState(SubobjectInfo);

			TArray<FWorkerComponentData> ActorSubobjectDatas =
				DataFactory.CreateComponentDatas(Subobject, SubobjectInfo, SubobjectRepChanges, SubobjectHandoverChanges, OutBytesWritten);

			// PRCOMMENT: This is a slight change from the previous flow, as we now add things to ACLs that we created in the data factory
			// previously, we did ForAllSchemaComps, and then if not invalid, add. Should be functionally the same...
			for (const FWorkerComponentData& ActorSubobjectData : ActorSubobjectDatas)
			{
				EntityComps.ComponentsToDelegateToAuthoritativeWorker.Add(ActorSubobjectData.component_id, ActorSubobjectData);
			}
		}
	}

	// Or if the subobject has handover properties, add it as well.
	// NOTE: this is only for subobjects that are a part of the CDO.
	// NOT dynamic subobjects which have been added before entity creation.
	for (auto& SubobjectInfoPair : Info.SubobjectInfo)
	{
		const FClassInfo& SubobjectInfo = SubobjectInfoPair.Value.Get();

		// Static subobjects aren't guaranteed to exist on actor instances, check they are present before adding to delegation component
		TWeakObjectPtr<UObject> WeakSubobject =
			PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Channel->GetEntityId(), SubobjectInfoPair.Key));
		if (!WeakSubobject.IsValid())
		{
			continue;
		}

		UObject* Subobject = WeakSubobject.Get();

		if (SubobjectInfo.SchemaComponents[SCHEMA_Handover] == SpatialConstants::INVALID_COMPONENT_ID)
		{
			continue;
		}

		// If it contains it, we've already created handover data for it.
		if (Channel->ReplicationMap.Contains(Subobject))
		{
			continue;
		}

		FHandoverChangeState SubobjectHandoverChanges = Channel->CreateInitialHandoverChangeState(SubobjectInfo);

		FWorkerComponentData SubobjectHandoverData = DataFactory.CreateHandoverComponentData(
			SubobjectInfo.SchemaComponents[SCHEMA_Handover], Subobject, SubobjectInfo, SubobjectHandoverChanges, OutBytesWritten);

		EntityComps.ComponentsToDelegateToAuthoritativeWorker.Add(SubobjectHandoverData.component_id, SubobjectHandoverData);
	}
}

TArray<FWorkerComponentData> EntityFactory::CreateEntityComponents(USpatialActorChannel* Channel,
																   FRPCsOnEntityCreationMap& OutgoingOnCreateEntityRPCs,
																   uint32& OutBytesWritten)
{
	EntityComponents EntityComps = CreateSkeletonEntityComponents(Channel->Actor);
	WriteUnrealComponents(EntityComps, Channel, OutgoingOnCreateEntityRPCs, OutBytesWritten);
	WriteLBComponents(EntityComps, Channel->Actor);
	EntityComps.ShiftToComponentDatas();
	return EntityComps.ComponentDatas;
}

// This method should be called once all the components besides ComponentPresence have been added to the
// ComponentDatas list.
TArray<Worker_ComponentId> EntityFactory::GetComponentPresenceList(const TArray<FWorkerComponentData>& ComponentDatas)
{
	TArray<Worker_ComponentId> ComponentPresenceList;
	ComponentPresenceList.SetNum(ComponentDatas.Num() + 1);
	for (int i = 0; i < ComponentDatas.Num(); i++)
	{
		ComponentPresenceList[i] = ComponentDatas[i].component_id;
	}
	ComponentPresenceList[ComponentDatas.Num()] = SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID;
	return ComponentPresenceList;
}

TArray<FWorkerComponentData> EntityFactory::CreateTombstoneEntityComponents(AActor* Actor)
{
	check(Actor->IsNetStartupActor());

	const UClass* Class = Actor->GetClass();

	// Get a stable object ref.
	FUnrealObjectRef OuterObjectRef = PackageMap->GetUnrealObjectRefFromObject(Actor->GetOuter());
	if (OuterObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
	{
		const FNetworkGUID NetGUID = PackageMap->ResolveStablyNamedObject(Actor->GetOuter());
		OuterObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
	}

	// No path in SpatialOS should contain a PIE prefix.
	FString TempPath = Actor->GetFName().ToString();
	GEngine->NetworkRemapPath(NetDriver, TempPath, false /*bIsReading*/);
	const TSchemaOption<FUnrealObjectRef> StablyNamedObjectRef = FUnrealObjectRef(0, 0, TempPath, OuterObjectRef, true);

	TArray<FWorkerComponentData> Components;
	Components.Add(Position(Coordinates::FromFVector(GetActorSpatialPosition(Actor))).CreateComponentData());
	Components.Add(Metadata(Class->GetName()).CreateComponentData());
	Components.Add(UnrealMetadata(StablyNamedObjectRef, Class->GetPathName(), true).CreateComponentData());
	Components.Add(Tombstone().CreateComponentData());
	Components.Add(AuthorityDelegation().CreateComponentData());

	Worker_ComponentId ActorInterestComponentId = ClassInfoManager->ComputeActorInterestComponentId(Actor);
	if (ActorInterestComponentId != SpatialConstants::INVALID_COMPONENT_ID)
	{
		Components.Add(ComponentFactory::CreateEmptyComponentData(ActorInterestComponentId));
	}

	if (ShouldActorHaveVisibleComponent(Actor))
	{
		Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::VISIBLE_COMPONENT_ID));
	}

	if (!Class->HasAnySpatialClassFlags(SPATIALCLASS_NotPersistent))
	{
		Components.Add(Persistence().CreateComponentData());
	}

	// Add Actor completeness tags.
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ACTOR_NON_AUTH_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::LB_TAG_COMPONENT_ID));

	return Components;
}

TArray<FWorkerComponentData> EntityFactory::CreatePartitionEntityComponents(const Worker_EntityId EntityId,
																			const InterestFactory* InterestFactory,
																			const UAbstractLBStrategy* LbStrategy,
																			VirtualWorkerId VirtualWorker, bool bDebugContextValid)
{
	AuthorityDelegationMap DelegationMap;
	DelegationMap.Add(SpatialConstants::WELL_KNOWN_COMPONENT_SET_ID, EntityId);
	DelegationMap.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, EntityId);

	TArray<FWorkerComponentData> Components;
	Components.Add(Position().CreateComponentData());
	Components.Add(Metadata(FString::Format(TEXT("PartitionEntity:{0}"), { VirtualWorker })).CreateComponentData());
	Components.Add(InterestFactory->CreatePartitionInterest(LbStrategy, VirtualWorker, bDebugContextValid).CreateComponentData());
	Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());
	Components.Add(ComponentPresence(GetComponentPresenceList(Components)).CreateComponentData());
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));

	return Components;
}

} // namespace SpatialGDK
