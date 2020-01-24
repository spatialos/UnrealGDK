// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
 
#include "Utils/EntityFactory.h"
 
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/SpatialRPCService.h"
#include "Schema/AlwaysRelevant.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/Heartbeat.h"
#include "Schema/ClientRPCEndpointLegacy.h"
#include "Schema/ServerRPCEndpointLegacy.h"
#include "Schema/RPCPayload.h"
#include "Schema/Singleton.h"
#include "Schema/SpatialDebugging.h"
#include "Schema/SpawnData.h"
#include "Utils/ComponentFactory.h"
#include "Utils/InspectionColors.h"
#include "Utils/InterestFactory.h"
#include "Utils/SpatialActorUtils.h"
#include "Utils/SpatialDebugger.h"

#include "Engine.h"

namespace SpatialGDK
{

EntityFactory::EntityFactory(USpatialNetDriver* InNetDriver, USpatialPackageMapClient* InPackageMap, USpatialClassInfoManager* InClassInfoManager, SpatialRPCService* InRPCService)
	: NetDriver(InNetDriver)
	, PackageMap(InPackageMap)
	, ClassInfoManager(InClassInfoManager)
	, RPCService(InRPCService)
{ }
 
TArray<Worker_ComponentData> EntityFactory::CreateEntityComponents(USpatialActorChannel* Channel, FRPCsOnEntityCreationMap& OutgoingOnCreateEntityRPCs)
{
	AActor* Actor = Channel->Actor;
	UClass* Class = Actor->GetClass();
	Worker_EntityId EntityId = Channel->GetEntityId();

	FString ClientWorkerAttribute = GetOwnerWorkerAttribute(Actor);

	WorkerRequirementSet AnyServerRequirementSet;
	WorkerRequirementSet AnyServerOrClientRequirementSet = { SpatialConstants::UnrealClientAttributeSet };

	WorkerAttributeSet OwningClientAttributeSet = { ClientWorkerAttribute };

	WorkerRequirementSet AnyServerOrOwningClientRequirementSet = { OwningClientAttributeSet };
	WorkerRequirementSet OwningClientOnlyRequirementSet = { OwningClientAttributeSet };

	for (const FName& WorkerType : GetDefault<USpatialGDKSettings>()->ServerWorkerTypes)
	{
		WorkerAttributeSet ServerWorkerAttributeSet = { WorkerType.ToString() };

		AnyServerRequirementSet.Add(ServerWorkerAttributeSet);
		AnyServerOrClientRequirementSet.Add(ServerWorkerAttributeSet);
		AnyServerOrOwningClientRequirementSet.Add(ServerWorkerAttributeSet);
	}

	// Add Zoning Attribute if we are using the load balancer.
	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();
	if (SpatialSettings->bEnableUnrealLoadBalancer)
	{
		WorkerAttributeSet ZoningAttributeSet = { SpatialConstants::ZoningAttribute };
		AnyServerRequirementSet.Add(ZoningAttributeSet);
		AnyServerOrClientRequirementSet.Add(ZoningAttributeSet);
		AnyServerOrOwningClientRequirementSet.Add(ZoningAttributeSet);
	}

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

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Class);

	const WorkerAttributeSet WorkerAttribute{ Info.WorkerType.ToString() };
	const WorkerRequirementSet AuthoritativeWorkerRequirementSet = { WorkerAttribute };

	WriteAclMap ComponentWriteAcl;
	ComponentWriteAcl.Add(SpatialConstants::POSITION_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::INTEREST_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::SPAWN_DATA_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	ComponentWriteAcl.Add(SpatialConstants::DORMANT_COMPONENT_ID, AuthoritativeWorkerRequirementSet);

	if (SpatialSettings->bUseRPCRingBuffers && RPCService != nullptr)
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

	if (SpatialSettings->bEnableUnrealLoadBalancer)
	{
		const WorkerAttributeSet ACLAttributeSet = { SpatialConstants::ZoningAttribute };
		const WorkerRequirementSet ACLRequirementSet = { ACLAttributeSet };
		ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, ACLRequirementSet);
		ComponentWriteAcl.Add(SpatialConstants::AUTHORITY_INTENT_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	}
	else
	{
		const WorkerAttributeSet ACLAttributeSet = { Info.WorkerType.ToString() };
		const WorkerRequirementSet ACLRequirementSet = { ACLAttributeSet };
		ComponentWriteAcl.Add(SpatialConstants::ENTITY_ACL_COMPONENT_ID, ACLRequirementSet);
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

	ComponentWriteAcl.Add(SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID, AuthoritativeWorkerRequirementSet);

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

	TArray<Worker_ComponentData> ComponentDatas;
	ComponentDatas.Add(Position(Coordinates::FromFVector(GetActorSpatialPosition(Actor))).CreatePositionData());
	ComponentDatas.Add(Metadata(Class->GetName()).CreateMetadataData());
	ComponentDatas.Add(SpawnData(Actor).CreateSpawnDataData());
	ComponentDatas.Add(UnrealMetadata(StablyNamedObjectRef, ClientWorkerAttribute, Class->GetPathName(), bNetStartup).CreateUnrealMetadataData());

	if (!Class->HasAnySpatialClassFlags(SPATIALCLASS_NotPersistent))
	{
		ComponentDatas.Add(Persistence().CreatePersistenceData());
	}

	if (SpatialSettings->bEnableUnrealLoadBalancer)
	{
		ComponentDatas.Add(AuthorityIntent::CreateAuthorityIntentData(NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId()));
	}

	if (NetDriver->SpatialDebugger != nullptr)
	{
		if (SpatialSettings->bEnableUnrealLoadBalancer)
		{
			check(NetDriver->VirtualWorkerTranslator != nullptr);

			VirtualWorkerId IntentVirtualWorkerId = NetDriver->VirtualWorkerTranslator->GetLocalVirtualWorkerId();

			const PhysicalWorkerName* PhysicalWorkerName = NetDriver->VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(IntentVirtualWorkerId);
			FColor InvalidServerTintColor = NetDriver->SpatialDebugger->InvalidServerTintColor;
			FColor IntentColor = PhysicalWorkerName == nullptr ? InvalidServerTintColor : SpatialGDK::GetColorForWorkerName(*PhysicalWorkerName);

			SpatialDebugging DebuggingInfo(SpatialConstants::INVALID_VIRTUAL_WORKER_ID, InvalidServerTintColor, IntentVirtualWorkerId, IntentColor, false);
			ComponentDatas.Add(DebuggingInfo.CreateSpatialDebuggingData());
		}

		ComponentWriteAcl.Add(SpatialConstants::SPATIAL_DEBUGGING_COMPONENT_ID, AuthoritativeWorkerRequirementSet);
	}

	if (Class->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
	{
		ComponentDatas.Add(Singleton().CreateSingletonData());
	}

	if (Actor->bAlwaysRelevant)
	{
		ComponentDatas.Add(AlwaysRelevant().CreateData());
	}

	if (Actor->NetDormancy >= DORM_DormantAll)
	{
		ComponentDatas.Add(Dormant().CreateData());
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

	TArray<Worker_ComponentData> DynamicComponentDatas = DataFactory.CreateComponentDatas(Actor, Info, InitialRepChanges, InitialHandoverChanges);
	ComponentDatas.Append(DynamicComponentDatas);

	InterestFactory InterestDataFactory(Actor, Info, ClassInfoManager, PackageMap);
	ComponentDatas.Add(InterestDataFactory.CreateInterestData());

	if (SpatialSettings->bUseRPCRingBuffers && RPCService != nullptr)
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
				const FClassInfo* SubobjectInfo = Channel->TryResolveNewDynamicSubobjectAndGetClassInfo(Subobject);

				if (SubobjectInfo == nullptr)
				{
					// This is a failure but there is already a log inside TryResolveNewDynamicSubbojectAndGetClassInfo
					continue;
				}

				ForAllSchemaComponentTypes([&](ESchemaComponentType Type)
				{
					if (SubobjectInfo->SchemaComponents[Type] != SpatialConstants::INVALID_COMPONENT_ID)
					{
						ComponentWriteAcl.Add(SubobjectInfo->SchemaComponents[Type], AuthoritativeWorkerRequirementSet);
					}
				});
			}

			const FClassInfo& SubobjectInfo = ClassInfoManager->GetOrCreateClassInfoByObject(Subobject);

			FRepChangeState SubobjectRepChanges = Channel->CreateInitialRepChangeState(Subobject);
			FHandoverChangeState SubobjectHandoverChanges = Channel->CreateInitialHandoverChangeState(SubobjectInfo);

			TArray<Worker_ComponentData> ActorSubobjectDatas = DataFactory.CreateComponentDatas(Subobject, SubobjectInfo, SubobjectRepChanges, SubobjectHandoverChanges);
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

		TraceKey LatencyKey; // Currently untracked. Will be dealt with by UNR-2726 
		Worker_ComponentData SubobjectHandoverData = DataFactory.CreateHandoverComponentData(SubobjectInfo.SchemaComponents[SCHEMA_Handover], Subobject, SubobjectInfo, SubobjectHandoverChanges, LatencyKey);
		ComponentDatas.Add(SubobjectHandoverData);

		ComponentWriteAcl.Add(SubobjectInfo.SchemaComponents[SCHEMA_Handover], AuthoritativeWorkerRequirementSet);
	}

	ComponentDatas.Add(EntityAcl(ReadAcl, ComponentWriteAcl).CreateEntityAclData());
 
	return ComponentDatas;
}
}
