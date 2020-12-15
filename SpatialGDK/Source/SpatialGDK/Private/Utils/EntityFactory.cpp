// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/EntityFactory.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/RPCs/SpatialRPCService.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/Heartbeat.h"
#include "Schema/NetOwningClientWorker.h"
#include "Schema/RPCPayload.h"
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
#include "Runtime/Launch/Resources/Version.h"

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

TArray<FWorkerComponentData> EntityFactory::CreateEntityComponents(USpatialActorChannel* Channel, uint32& OutBytesWritten)
{
	AActor* Actor = Channel->Actor;
	UClass* Class = Actor->GetClass();
	Worker_EntityId EntityId = Channel->GetEntityId();

	AuthorityDelegationMap DelegationMap{};
	const Worker_PartitionId AuthoritativeServerPartitionId = NetDriver->VirtualWorkerTranslator->GetClaimedPartitionId();
	const Worker_PartitionId AuthoritativeClientPartitionId = GetConnectionOwningPartitionId(Actor);
	DelegationMap.Add(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, AuthoritativeServerPartitionId);
	DelegationMap.Add(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, AuthoritativeClientPartitionId);

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Class);

	// Add Load Balancer Attribute. If this is a single worker deployment, this will be just be the single worker.
	const VirtualWorkerId IntendedVirtualWorkerId = NetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId();
	checkf(IntendedVirtualWorkerId != SpatialConstants::INVALID_VIRTUAL_WORKER_ID,
		   TEXT("Load balancing strategy provided invalid local virtual worker ID during Actor spawn. "
				"Actor: %s. Strategy: %s"),
		   *Actor->GetName(), *NetDriver->LoadBalanceStrategy->GetName());

	Worker_ComponentId ActorInterestComponentId = ClassInfoManager->ComputeActorInterestComponentId(Actor);

	for (auto& SubobjectInfoPair : Info.SubobjectInfo)
	{
		const FClassInfo& SubobjectInfo = SubobjectInfoPair.Value.Get();

		// Static subobjects aren't guaranteed to exist on actor instances, check they are present before adding to delegation component
		TWeakObjectPtr<UObject> Subobject = PackageMap->GetObjectFromUnrealObjectRef(FUnrealObjectRef(EntityId, SubobjectInfoPair.Key));
		if (!Subobject.IsValid())
		{
			continue;
		}
	}

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
#if ENGINE_MINOR_VERSION >= 26
		GEngine->NetworkRemapPath(NetDriver->GetSpatialOSNetConnection(), TempPath, false /*bIsReading*/);
#else
		GEngine->NetworkRemapPath(NetDriver, TempPath, false /*bIsReading*/);
#endif

		StablyNamedObjectRef = FUnrealObjectRef(0, 0, TempPath, OuterObjectRef, true);
		bNetStartup = Actor->bNetStartup;
	}

	TArray<FWorkerComponentData> ComponentDatas;
	ComponentDatas.Add(Position(Coordinates::FromFVector(GetActorSpatialPosition(Actor))).CreatePositionData());
	ComponentDatas.Add(Metadata(Class->GetName()).CreateMetadataData());
	ComponentDatas.Add(SpawnData(Actor).CreateSpawnDataData());
	ComponentDatas.Add(UnrealMetadata(StablyNamedObjectRef, Class->GetPathName(), bNetStartup).CreateUnrealMetadataData());
	ComponentDatas.Add(NetOwningClientWorker(AuthoritativeClientPartitionId).CreateNetOwningClientWorkerData());
	ComponentDatas.Add(AuthorityIntent::CreateAuthorityIntentData(IntendedVirtualWorkerId));
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID));

	if (ShouldActorHaveVisibleComponent(Actor))
	{
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::VISIBLE_COMPONENT_ID));
	}

	if (!Class->HasAnySpatialClassFlags(SPATIALCLASS_NotPersistent))
	{
		ComponentDatas.Add(Persistence().CreatePersistenceData());
	}

#if !UE_BUILD_SHIPPING
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
		ComponentDatas.Add(DebuggingInfo.CreateSpatialDebuggingData());
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

	TArray<FWorkerComponentData> DynamicComponentDatas =
		DataFactory.CreateComponentDatas(Actor, Info, InitialRepChanges, InitialHandoverChanges, OutBytesWritten);

	ComponentDatas.Append(DynamicComponentDatas);

	ComponentDatas.Add(NetDriver->InterestFactory->CreateInterestData(Actor, Info, EntityId));

	Channel->SetNeedOwnerInterestUpdate(!NetDriver->InterestFactory->DoOwnersHaveEntityId(Actor));

	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID));

	checkf(RPCService != nullptr, TEXT("Attempting to create an entity with a null RPCService."));
	ComponentDatas.Append(RPCService->GetRPCComponentsOnEntityCreation(EntityId));

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

			ComponentDatas.Append(ActorSubobjectDatas);
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

		ComponentDatas.Add(SubobjectHandoverData);
	}

	ComponentDatas.Add(AuthorityDelegation(DelegationMap).CreateAuthorityDelegationData());

	// Add Actor completeness tags.
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID));
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ACTOR_NON_AUTH_TAG_COMPONENT_ID));
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::LB_TAG_COMPONENT_ID));

	return ComponentDatas;
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
#if ENGINE_MINOR_VERSION >= 26
	GEngine->NetworkRemapPath(NetDriver->GetSpatialOSNetConnection(), TempPath, false /*bIsReading*/);
#else
	GEngine->NetworkRemapPath(NetDriver, TempPath, false /*bIsReading*/);
#endif
	const TSchemaOption<FUnrealObjectRef> StablyNamedObjectRef = FUnrealObjectRef(0, 0, TempPath, OuterObjectRef, true);

	TArray<FWorkerComponentData> Components;
	Components.Add(Position(Coordinates::FromFVector(GetActorSpatialPosition(Actor))).CreatePositionData());
	Components.Add(Metadata(Class->GetName()).CreateMetadataData());
	Components.Add(UnrealMetadata(StablyNamedObjectRef, Class->GetPathName(), true).CreateUnrealMetadataData());
	Components.Add(Tombstone().CreateData());
	Components.Add(AuthorityDelegation().CreateAuthorityDelegationData());

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
		Components.Add(Persistence().CreatePersistenceData());
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
	DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, EntityId);

	TArray<FWorkerComponentData> Components;
	Components.Add(Position().CreatePositionData());
	Components.Add(Metadata(FString::Format(TEXT("PartitionEntity:{0}"), { VirtualWorker })).CreateMetadataData());
	Components.Add(InterestFactory->CreatePartitionInterest(LbStrategy, VirtualWorker, bDebugContextValid).CreateInterestData());
	Components.Add(AuthorityDelegation(DelegationMap).CreateAuthorityDelegationData());
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));

	return Components;
}

} // namespace SpatialGDK
