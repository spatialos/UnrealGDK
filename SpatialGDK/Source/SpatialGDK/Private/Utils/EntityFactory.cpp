// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/EntityFactory.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/SpatialRPCService.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/ComponentPresence.h"
#include "Schema/Heartbeat.h"
#include "Schema/ClientRPCEndpointLegacy.h"
#include "Schema/ServerRPCEndpointLegacy.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/RPCPayload.h"
#include "Schema/SpatialDebugging.h"
#include "Schema/SpawnData.h"
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

EntityFactory::EntityFactory(USpatialNetDriver* InNetDriver, USpatialPackageMapClient* InPackageMap, USpatialClassInfoManager* InClassInfoManager, SpatialRPCService* InRPCService)
	: NetDriver(InNetDriver)
	, PackageMap(InPackageMap)
	, ClassInfoManager(InClassInfoManager)
	, RPCService(InRPCService)
{ }

TArray<FWorkerComponentData> EntityFactory::CreateEntityComponents(USpatialActorChannel* Channel, FRPCsOnEntityCreationMap& OutgoingOnCreateEntityRPCs, uint32& OutBytesWritten)
{
	AActor* Actor = Channel->Actor;
	UClass* Class = Actor->GetClass();
	Worker_EntityId EntityId = Channel->GetEntityId();

	FString ClientWorkerAttribute = GetConnectionOwningWorkerId(Actor);

	WorkerRequirementSet AnyServerRequirementSet = { SpatialConstants::UnrealServerAttributeSet };
	WorkerRequirementSet AnyServerOrClientRequirementSet = { SpatialConstants::UnrealServerAttributeSet, SpatialConstants::UnrealClientAttributeSet };

	WorkerAttributeSet OwningClientAttributeSet = { ClientWorkerAttribute };

	WorkerRequirementSet AnyServerOrOwningClientRequirementSet = { SpatialConstants::UnrealServerAttributeSet, OwningClientAttributeSet };
	WorkerRequirementSet OwningClientOnlyRequirementSet = { OwningClientAttributeSet };

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Class);

	// Add Load Balancer Attribute. If this is a single worker deployment, this will be just be the single worker.
	WorkerAttributeSet WorkerAttributeOrSpecificWorker = SpatialConstants::UnrealServerAttributeSet;
	const VirtualWorkerId IntendedVirtualWorkerId = NetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId();
	if (IntendedVirtualWorkerId != SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	{
		const PhysicalWorkerName* IntendedAuthoritativePhysicalWorkerName = NetDriver->VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(IntendedVirtualWorkerId);
		WorkerAttributeOrSpecificWorker = { FString::Format(TEXT("workerId:{0}"), { *IntendedAuthoritativePhysicalWorkerName }) };
	}
	else
	{
		UE_LOG(LogEntityFactory, Error, TEXT("Load balancing strategy provided invalid local virtual worker ID during Actor spawn. Actor: %s. Strategy: %s"), *Actor->GetName(), *NetDriver->LoadBalanceStrategy->GetName());
	}

	const WorkerRequirementSet AuthoritativeWorkerRequirementSet = { WorkerAttributeOrSpecificWorker };

	WorkerRequirementSet ReadAcl;
	if (Class->HasAnySpatialClassFlags(SPATIALCLASS_ServerOnly))
	{
		ReadAcl = AnyServerRequirementSet;
	}
	else if (Actor->IsA<APlayerController>())
	{
		ReadAcl = AnyServerOrOwningClientRequirementSet;
	}
	else
	{
		ReadAcl = AnyServerOrClientRequirementSet;
	}

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::INTEREST_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::SPAWN_DATA_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::DORMANT_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::COMPONENT_PRESENCE_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::UNREAL_METADATA_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::NET_OWNING_CLIENT_WORKER_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, AnyServerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID, AuthoritativeWorkerRequirementSet);

	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();
	if (SpatialSettings->UseRPCRingBuffer() && RPCService != nullptr)
	{
		ComponentWriteAcl.Add(SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, OwningClientOnlyRequirementSet);
		ComponentWriteAcl.Add(SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
		ComponentWriteAcl.Add(SpatialConstants::MULTICAST_RPCS_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	}
	else
	{
		ComponentWriteAcl.Add(SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_LEGACY, AuthoritativeWorkerRequirementSet);
		ComponentWriteAcl.Add(SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_LEGACY, AuthoritativeWorkerRequirementSet);
		ComponentWriteAcl.Add(SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_LEGACY, OwningClientOnlyRequirementSet);

		// If there are pending RPCs, add this component.
		if (OutgoingOnCreateEntityRPCs.Contains(Actor))
		{
			ComponentWriteAcl.Add(SpatialConstants::RPCS_ON_ENTITY_CREATION_ID, AuthoritativeWorkerRequirementSet);
		}
	}

	if (Actor->IsNetStartupActor())
	{
		ComponentWriteAcl.Add(SpatialConstants::TOMBSTONE_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	}

	// If Actor is a PlayerController, add the heartbeat component.
	if (Actor->IsA<APlayerController>())
	{
#if !UE_BUILD_SHIPPING
		ComponentWriteAcl.Add(SpatialConstants::DEBUG_METRICS_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
#endif // !UE_BUILD_SHIPPING
		ComponentWriteAcl.Add(SpatialConstants::HEARTBEAT_COMPONENT_ID, OwningClientOnlyRequirementSet);
	}

	// Add all Interest component IDs to allow us to change it if needed.
	ComponentWriteAcl.Add(SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	for (const auto ComponentId : ClassInfoManager->SchemaDatabase->NetCullDistanceComponentIds)
	{
		ComponentWriteAcl.Add(ComponentId, AuthoritativeWorkerRequirementSet);
	}

	Worker_ComponentId ActorInterestComponentId = ClassInfoManager->ComputeActorInterestComponentId(Actor);

	ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
	{
		Worker_ComponentId ComponentId = Info.SchemaComponents[Type];
		if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
		{
			return;
		}

		ComponentWriteAcl.Add(ComponentId, AuthoritativeWorkerRequirementSet);
	});

	for (auto& SubobjectInfoPair : Info.SubobjectInfo)
	{
		const FClassInfo& SubobjectInfo = SubobjectInfoPair.Value.Get();

		// Static subobjects aren't guaranteed to exist on actor instances, check they are present before adding write acls
		TWeakObjectPtr<UObject> Subobject = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(EntityId, SubobjectInfoPair.Key));
		if (!Subobject.IsValid())
		{
			continue;
		}

		ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
		{
			Worker_ComponentId ComponentId = SubobjectInfo.SchemaComponents[Type];
			if (ComponentId == SpatialConstants::INVALID_COMPONENT_ID)
			{
				return;
			}

			ComponentWriteAcl.Add(ComponentId, AuthoritativeWorkerRequirementSet);
		});
	}

	// We want to have a stably named ref if this is an Actor placed in the world.
	// We use this to indicate if a new Actor should be created, or to link a pre-existing Actor when receiving an AddEntityOp.
	// Previously, IsFullNameStableForNetworking was used but this was only true if bNetLoadOnClient=true.
	// Actors with bNetLoadOnClient=false also need a StablyNamedObjectRef for linking in the case of loading from a snapshot or the server crashes and restarts.
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

	TArray<FWorkerComponentData> ComponentDatas;
	ComponentDatas.Add(Position(Coordinates::FromFVector(GetActorSpatialPosition(Actor))).CreatePositionData());
	ComponentDatas.Add(Metadata(Class->GetName()).CreateMetadataData());
	ComponentDatas.Add(SpawnData(Actor).CreateSpawnDataData());
	ComponentDatas.Add(UnrealMetadata(StablyNamedObjectRef, Class->GetPathName(), bNetStartup).CreateUnrealMetadataData());
	ComponentDatas.Add(NetOwningClientWorker(GetConnectionOwningWorkerId(Channel->Actor)).CreateNetOwningClientWorkerData());
	ComponentDatas.Add(AuthorityIntent::CreateAuthorityIntentData(IntendedVirtualWorkerId));

	if (!Class->HasAnySpatialClassFlags(SPATIALCLASS_NotPersistent))
	{
		ComponentDatas.Add(Persistence().CreatePersistenceData());
	}

#if !UE_BUILD_SHIPPING
	if (NetDriver->SpatialDebugger != nullptr)
	{
		check(NetDriver->VirtualWorkerTranslator != nullptr);

		const PhysicalWorkerName* PhysicalWorkerName = NetDriver->VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(IntendedVirtualWorkerId);
		FColor InvalidServerTintColor = NetDriver->SpatialDebugger->InvalidServerTintColor;
		FColor IntentColor = PhysicalWorkerName != nullptr ? SpatialGDK::GetColorForWorkerName(*PhysicalWorkerName) : InvalidServerTintColor;

		const bool bIsLocked = NetDriver->LockingPolicy->IsLocked(Actor);

		SpatialDebugging DebuggingInfo(SpatialConstants::INVALID_VIRTUAL_WORKER_ID, InvalidServerTintColor, IntendedVirtualWorkerId, IntentColor, bIsLocked);
		ComponentDatas.Add(DebuggingInfo.CreateSpatialDebuggingData());
		ComponentWriteAcl.Add(SpatialConstants::SPATIAL_DEBUGGING_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	}
#endif

	if (ActorInterestComponentId != SpatialConstants::INVALID_COMPONENT_ID)
	{
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(ActorInterestComponentId));
	}

	if (Actor->NetDormancy >= DORM_DormantAll)
	{
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::DORMANT_COMPONENT_ID));
	}

	if (Actor->IsA<APlayerController>())
	{
#if !UE_BUILD_SHIPPING
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::DEBUG_METRICS_COMPONENT_ID));
#endif // !UE_BUILD_SHIPPING
		ComponentDatas.Add(Heartbeat().CreateHeartbeatData());
	}

	USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(Actor);

	ComponentFactory DataFactory(false, NetDriver, Tracer);

	FRepChangeState InitialRepChanges = Channel->CreateInitialRepChangeState(Actor);
	FHandoverChangeState InitialHandoverChanges = Channel->CreateInitialHandoverChangeState(Info);

	TArray<FWorkerComponentData> DynamicComponentDatas = DataFactory.CreateComponentDatas(Actor, Info, InitialRepChanges, InitialHandoverChanges, OutBytesWritten);

	ComponentDatas.Append(DynamicComponentDatas);

	bool bOwnerReady;
	ComponentDatas.Add(NetDriver->InterestFactory->CreateInterestData(Actor, Info, EntityId, bOwnerReady));

	Channel->SetNeedOwnerInterestUpdate(!bOwnerReady);

	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID));

	if (SpatialSettings->UseRPCRingBuffer() && RPCService != nullptr)
	{
		ComponentDatas.Append(RPCService->GetRPCComponentsOnEntityCreation(EntityId));
	}
	else
	{
		ComponentDatas.Add(ClientRPCEndpointLegacy().CreateRPCEndpointData());
		ComponentDatas.Add(ServerRPCEndpointLegacy().CreateRPCEndpointData());
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_LEGACY));

		if (RPCsOnEntityCreation* QueuedRPCs = OutgoingOnCreateEntityRPCs.Find(Actor))
		{
			if (QueuedRPCs->HasRPCPayloadData())
			{
				ComponentDatas.Add(QueuedRPCs->CreateRPCPayloadData());
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

			ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
			{
				if (SubobjectInfo.SchemaComponents[Type] != SpatialConstants::INVALID_COMPONENT_ID)
				{
					ComponentWriteAcl.Add(SubobjectInfo.SchemaComponents[Type], AuthoritativeWorkerRequirementSet);
				}
			});

			TArray<FWorkerComponentData> ActorSubobjectDatas = DataFactory.CreateComponentDatas(Subobject, SubobjectInfo, SubobjectRepChanges, SubobjectHandoverChanges, OutBytesWritten);

			ComponentDatas.Append(ActorSubobjectDatas);
		}
	}

	// Or if the subobject has handover properties, add it as well.
	// NOTE: this is only for subobjects that are a part of the CDO.
	// NOT dynamic subobjects which have been added before entity creation.
	for (auto& SubobjectInfoPair : Info.SubobjectInfo)
	{
		const FClassInfo& SubobjectInfo = SubobjectInfoPair.Value.Get();

		// Static subobjects aren't guaranteed to exist on actor instances, check they are present before adding write acls
		TWeakObjectPtr<UObject> WeakSubobject = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(Channel->GetEntityId(), SubobjectInfoPair.Key));
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

		FWorkerComponentData SubobjectHandoverData = DataFactory.CreateHandoverComponentData(SubobjectInfo.SchemaComponents[SCHEMA_Handover], Subobject, SubobjectInfo, SubobjectHandoverChanges, OutBytesWritten);

		ComponentDatas.Add(SubobjectHandoverData);

		ComponentWriteAcl.Add(SubobjectInfo.SchemaComponents[SCHEMA_Handover], AuthoritativeWorkerRequirementSet);
	}

	ComponentDatas.Add(EntityAcl(ReadAcl, ComponentWriteAcl).CreateEntityAclData());

	return ComponentDatas;
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

	// Construct an ACL for a read-only entity.
	WorkerRequirementSet AnyServerRequirementSet = { SpatialConstants::UnrealServerAttributeSet };
	WorkerRequirementSet AnyServerOrClientRequirementSet = { SpatialConstants::UnrealServerAttributeSet, SpatialConstants::UnrealClientAttributeSet };

	WorkerRequirementSet ReadAcl;
	if (Class->HasAnySpatialClassFlags(SPATIALCLASS_ServerOnly))
	{
		ReadAcl = AnyServerRequirementSet;
	}
	else
	{
		ReadAcl = AnyServerOrClientRequirementSet;
	}

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
	Components.Add(Position(Coordinates::FromFVector(GetActorSpatialPosition(Actor))).CreatePositionData());
	Components.Add(Metadata(Class->GetName()).CreateMetadataData());
	Components.Add(UnrealMetadata(StablyNamedObjectRef, Class->GetPathName(), true).CreateUnrealMetadataData());
	Components.Add(Tombstone().CreateData());
	Components.Add(EntityAcl(ReadAcl, WriteAclMap()).CreateEntityAclData());

	Worker_ComponentId ActorInterestComponentId = ClassInfoManager->ComputeActorInterestComponentId(Actor);
	if (ActorInterestComponentId != SpatialConstants::INVALID_COMPONENT_ID)
	{
		Components.Add(ComponentFactory::CreateEmptyComponentData(ActorInterestComponentId));
	}

	if (!Class->HasAnySpatialClassFlags(SPATIALCLASS_NotPersistent))
	{
		Components.Add(Persistence().CreatePersistenceData());
	}

	return Components;
}

}  // namespace SpatialGDK
