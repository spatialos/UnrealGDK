// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/EntityFactory.h"

#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetConnection.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialNetDriverRPC.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/ActorGroupWriter.h"
#include "Interop/ActorSetWriter.h"
#include "Interop/RPCs/SpatialRPCService.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/ActorGroupMember.h"
#include "Schema/ActorOwnership.h"
#include "Schema/ActorSetMember.h"
#include "Schema/AuthorityIntent.h"
#include "Schema/NetOwningClientWorker.h"
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
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Runtime/Launch/Resources/Version.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategoryReplicator.h"
#endif

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

FUnrealObjectRef GetStablyNamedObjectRef(const UObject* Object)
{
	if (Object == nullptr)
	{
		return FUnrealObjectRef::NULL_OBJECT_REF;
	}

	// No path in SpatialOS should contain a PIE prefix.
	FString TempPath = Object->GetFName().ToString();
	TempPath = UWorld::RemovePIEPrefix(TempPath);

	return FUnrealObjectRef(0, 0, TempPath, GetStablyNamedObjectRef(Object->GetOuter()), true);
}

TArray<FWorkerComponentData> EntityFactory::CreateMinimalEntityComponents(AActor* Actor)
{
	UClass* Class = Actor->GetClass();

	TArray<FWorkerComponentData> ComponentDatas;
	ComponentDatas.Add(Position(Coordinates::FromFVector(GetActorSpatialPosition(Actor))).CreateComponentData());
	ComponentDatas.Add(Metadata(Class->GetName()).CreateComponentData());
	ComponentDatas.Add(SpawnData(Actor).CreateComponentData());

	if (!Class->HasAnySpatialClassFlags(SPATIALCLASS_NotPersistent))
	{
		ComponentDatas.Add(Persistence().CreateComponentData());
	}

	ComponentDatas.Add(CreateMetadata(*Actor).CreateComponentData());

	// Add Actor completeness tags.
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ACTOR_AUTH_TAG_COMPONENT_ID));
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ACTOR_TAG_COMPONENT_ID));
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID));
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::LB_TAG_COMPONENT_ID));
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::STRATEGYWORKER_TAG_COMPONENT_ID));
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID));
	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_DEBUG_TAG_COMPONENT_ID));

	// if (Class->IsLightweightActor())
	if (Class->IsChildOf<ACharacter>()) // TODO: Add a way to mark classes as lightweight
										// Alternatively could move this to WriteUnrealComponents after checking for lightweight comps.
	{
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::LIGHTWEIGHT_ENTITY_COMPONENT_ID));
	}

#if WITH_GAMEPLAY_DEBUGGER
	if (Actor->IsA<AGameplayDebuggerCategoryReplicator>())
	{
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_GAMEPLAY_DEBUGGER_COMPONENT_ID));
	}
#endif // WITH_GAMEPLAY_DEBUGGER

	return ComponentDatas;
}

UnrealMetadata EntityFactory::CreateMetadata(const AActor& InActor)
{
	UClass* Class = InActor.GetClass();

	// We want to have a stably named ref if this is an Actor placed in the world.
	// We use this to indicate if a new Actor should be created, or to link a pre-existing Actor when receiving an AddEntityOp.
	// We presume that all actors not in game worlds are in editor worlds, therefore the actors are stably named.
	// Previously, IsFullNameStableForNetworking was used but this was only true if bNetLoadOnClient=true.
	// Actors with bNetLoadOnClient=false also need a StablyNamedObjectRef for linking in the case of loading from a snapshot or the server
	// crashes and restarts.
	TSchemaOption<FUnrealObjectRef> StablyNamedObjectRef;
	TSchemaOption<bool> bNetStartup;
	if ((InActor.GetWorld() != nullptr && !InActor.GetWorld()->IsGameWorld()) || InActor.HasAnyFlags(RF_WasLoaded)
		|| InActor.IsNetStartupActor())
	{
		StablyNamedObjectRef = GetStablyNamedObjectRef(&InActor);
		bNetStartup = InActor.IsNetStartupActor();
	}
	return UnrealMetadata(StablyNamedObjectRef, Class->GetPathName(), bNetStartup);
}

void EntityFactory::WriteLBComponents(TArray<FWorkerComponentData>& ComponentDatas, AActor* Actor)
{
	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();
	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Actor->GetClass());

	const bool bUsesDistributedLoadBalancer =
		!(SpatialSettings->bRunStrategyWorker && NetDriver->LoadBalanceStrategy->IsStrategyWorkerAware());

	const Worker_PartitionId AuthoritativeServerPartitionId =
		bUsesDistributedLoadBalancer ? NetDriver->VirtualWorkerTranslator->GetClaimedPartitionId() : NetDriver->StagingPartitionId;
	const Worker_PartitionId AuthoritativeClientPartitionId = GetConnectionOwningPartitionId(Actor);

	// Add Load Balancer Attribute. If this is a single worker deployment, this will be just be the single worker.
	// NB : This is only used for the debugger.
	const VirtualWorkerId IntendedVirtualWorkerId =
		bUsesDistributedLoadBalancer ? NetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId() : 0;
	checkf(!bUsesDistributedLoadBalancer || IntendedVirtualWorkerId != SpatialConstants::INVALID_VIRTUAL_WORKER_ID,
		   TEXT("Load balancing strategy provided invalid local virtual worker ID during Actor spawn. "
				"Actor: %s. Strategy: %s"),
		   *Actor->GetName(), *NetDriver->LoadBalanceStrategy->GetName());

	AuthorityDelegationMap DelegationMap;

	DelegationMap.Add(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, AuthoritativeServerPartitionId);
	if (!SpatialSettings->bRunStrategyWorker)
	{
		DelegationMap.Add(SpatialConstants::LB_DELEGATION_AUTH_COMPONENT_SET_ID, AuthoritativeServerPartitionId);
	}
	else
	{
		DelegationMap.Add(SpatialConstants::LB_DELEGATION_AUTH_COMPONENT_SET_ID, SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID);
		DelegationMap.Add(SpatialConstants::STRATEGY_WORKER_AUTH_COMPONENT_SET_ID, SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID);
	}
	DelegationMap.Add(SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID, AuthoritativeClientPartitionId);
	DelegationMap.Add(SpatialConstants::ROUTING_WORKER_AUTH_COMPONENT_SET_ID, SpatialConstants::INITIAL_ROUTING_PARTITION_ENTITY_ID);

	// Add debugging utilities, if we are not compiling a shipping build
#if !UE_BUILD_SHIPPING
	if (NetDriver->SpatialDebugger != nullptr)
	{
		if (ensureAlwaysMsgf(SpatialSettings->bRunStrategyWorker || NetDriver->VirtualWorkerTranslator != nullptr,
							 TEXT("Failed to add debugging utilities. Translator was invalid")))
		{
			FString DummyWorkerName(TEXT("Beats me..."));
			const PhysicalWorkerName* PhysicalWorkerName =
				SpatialSettings->bRunStrategyWorker
					? &DummyWorkerName
					: NetDriver->VirtualWorkerTranslator->GetPhysicalWorkerForVirtualWorker(IntendedVirtualWorkerId);
			const FColor InvalidServerTintColor = NetDriver->SpatialDebugger->InvalidServerTintColor;
			const FColor IntentColor =
				PhysicalWorkerName != nullptr ? SpatialGDK::GetColorForWorkerName(*PhysicalWorkerName) : InvalidServerTintColor;

			const bool bIsLocked = NetDriver->LockingPolicy->IsLocked(Actor);

			SpatialDebugging DebuggingInfo(SpatialConstants::INVALID_VIRTUAL_WORKER_ID, InvalidServerTintColor, IntendedVirtualWorkerId,
										   IntentColor, bIsLocked);
			ComponentDatas.Add(DebuggingInfo.CreateComponentData());
		}
	}
#endif // !UE_BUILD_SHIPPING

	// Add actual load balancing components
	ComponentDatas.Add(NetOwningClientWorker(AuthoritativeClientPartitionId).CreateComponentData());
	if (SpatialSettings->bRunStrategyWorker)
	{
		ComponentDatas.Add(AuthorityIntentV2().CreateComponentData());
		ComponentDatas.Add(AuthorityIntentACK().CreateComponentData());
		if (NetDriver->LoadBalanceStrategy->IsStrategyWorkerAware())
		{
			TArray<SpatialGDK::ComponentData> DecoratorData = NetDriver->LoadBalanceStrategy->CreateStaticLoadBalancingData(*Actor);
			for (auto& Component : DecoratorData)
			{
				FWorkerComponentData DecoratorComponent;
				DecoratorComponent.component_id = Component.GetComponentId();
				DecoratorComponent.schema_type = MoveTemp(Component).Release();
				ComponentDatas.Add(DecoratorComponent);
			}
		}
	}
	if (bUsesDistributedLoadBalancer)
	{
		ComponentDatas.Add(AuthorityIntent(IntendedVirtualWorkerId).CreateComponentData());
	}

	ComponentDatas.Add(AuthorityDelegation(DelegationMap).CreateComponentData());

	// Actor set information.
	if (SpatialSettings->bRunStrategyWorker)
	{
		const auto AddComponentData = [&ComponentDatas](ComponentData Data) {
			Worker_ComponentData ComponentData;
			ComponentData.reserved = nullptr;
			ComponentData.component_id = Data.GetComponentId();
			ComponentData.schema_type = MoveTemp(Data).Release();
			ComponentData.user_handle = nullptr;

			ComponentDatas.Add(ComponentData);
		};

		AddComponentData(GetActorSetData(*NetDriver->PackageMap, *Actor).CreateComponentData());
	}
}

void EntityFactory::WriteRPCComponents(TArray<FWorkerComponentData>& ComponentDatas, USpatialActorChannel& Channel)
{
	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();

	checkf(RPCService != nullptr, TEXT("Attempting to create an entity with a null RPCService."));
	ComponentDatas.Append(RPCService->GetRPCComponentsOnEntityCreation(Channel.GetEntityId()));
	ComponentDatas.Append(NetDriver->RPCs->GetRPCComponentsOnEntityCreation(Channel.GetEntityId()));

	if (SpatialSettings->CrossServerRPCImplementation == ECrossServerRPCImplementation::RoutingWorker)
	{
		// Addition of CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID is handled in GetRPCComponentsOnEntityCreation
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CROSS_SERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID));
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CROSS_SERVER_RECEIVER_ENDPOINT_COMPONENT_ID));
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CROSS_SERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID));
	}
}

void EntityFactory::CheckStablyNamedActorPath(const TArray<FWorkerComponentData>& ComponentDatas, const AActor* Actor,
											  Worker_EntityId EntityId) const
{
	// Leaving this code block here to guarantee the resolution of outers of stably named actors
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

		// This block of code is just for checking purposes and should be removed in the future
		// TODO: UNR-4783
#if !UE_BUILD_SHIPPING
		const FWorkerComponentData* UnrealMetadataPtr = ComponentDatas.FindByPredicate([](const FWorkerComponentData& Data) {
			return Data.component_id == SpatialConstants::UNREAL_METADATA_COMPONENT_ID;
		});
		checkf(UnrealMetadataPtr,
			   TEXT("Entity being constructed from an actor did not have the UnrealMetadata component. This is forbidden."));
		UnrealMetadata Metadata(*UnrealMetadataPtr);
		FString TempPath = Actor->GetFName().ToString();
		GEngine->NetworkRemapPath(NetDriver->GetSpatialOSNetConnection(), TempPath, false /*bIsReading*/);
		FUnrealObjectRef Remapped = FUnrealObjectRef(0, 0, TempPath, OuterObjectRef, true);
		if (!Metadata.StablyNamedRef.IsSet() || *Metadata.StablyNamedRef != Remapped)
		{
			UE_LOG(LogEntityFactory, Error,
				   TEXT("When constructing an entity, the network remapped path for the stably named object path was not equal to the one "
						"constructed before. This is unexpected and could lead to bugs further down the line. Actor: %s, EntityId: %lld"),
				   *Actor->GetPathName(), EntityId);
		}

		if (!Metadata.bNetStartup.IsSet() || Metadata.bNetStartup != Actor->bNetStartup)
		{
			UE_LOG(LogEntityFactory, Error,
				   TEXT("When constructing an entity, the bNetStartup variable was not equal to the one constructed before. This is "
						"unexpected and could lead to bugs further down the line. Actor: %s, EntityId: %lld"),
				   *Actor->GetPathName(), EntityId);
		}
#endif // !UE_BUILD_SHIPPING
	}
}

void EntityFactory::WriteUnrealComponents(TArray<FWorkerComponentData>& ComponentDatas, USpatialActorChannel* Channel,
										  uint32& OutBytesWritten)
{
	AActor* Actor = Channel->Actor;
	UClass* Class = Actor->GetClass();
	Worker_EntityId EntityId = Channel->GetEntityId();

	const FClassInfo& Info = ClassInfoManager->GetOrCreateClassInfoByClass(Class);

	Worker_ComponentId ActorInterestComponentId = ClassInfoManager->ComputeActorInterestComponentId(Actor);
	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();

	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::MIGRATION_DIAGNOSTIC_COMPONENT_ID));

	if (Channel->GetInterestComponentId() != SpatialConstants::INVALID_COMPONENT_ID)
	{
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(Channel->GetInterestComponentId()));
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
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::PLAYER_CONTROLLER_COMPONENT_ID));
	}

	ComponentFactory DataFactory(false, NetDriver);

	FRepChangeState InitialRepChanges = Channel->CreateInitialRepChangeState(Actor);

	TArray<FWorkerComponentData> ActorDataComponents = DataFactory.CreateComponentDatas(Actor, Info, InitialRepChanges, OutBytesWritten);

	ComponentDatas.Append(ActorDataComponents);

	ComponentDatas.Add(Worker_ComponentData{ nullptr, ActorOwnership::ComponentId,
											 ActorOwnership::CreateFromActor(*Actor, *PackageMap).CreateComponentData().Release(),
											 nullptr });

	Channel->SetNeedOwnerInterestUpdate(!NetDriver->InterestFactory->DoOwnersHaveEntityId(Actor));

	ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID));

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

			TArray<FWorkerComponentData> ActorSubobjectDatas =
				DataFactory.CreateComponentDatas(Subobject, SubobjectInfo, SubobjectRepChanges, OutBytesWritten);
			ComponentDatas.Append(ActorSubobjectDatas);
		}
	}

#if 0
	// Iterate over lightweight components
	TArray<ULightweightComponent*> LightweightComponents;
	Actor->GetComponents<ULightweightComponent>(LightweightComponents);
	for (auto Component : LightweightComponents)
	{
		const FLightweightComponentInfo& ComponentInfo = ClassInfoManager->GetOrCreateLightweightComponentInfoByObject(Component);

		// TODO: What do about rep changes?
		// Probably want to avoid actually having these components replicated in the usual sense,
		// but then need to track changes manually?
		FRepChangeState ComponentRepChanges = Channel->CreateInitialLightweightChangeState(Component);

		FWorkerComponentData ComponentData = DataFactory.CreateLightweightComponentData(Component, ComponentInfo, OutBytesWritten);
		ComponentDatas.Add(ComponentData);
	}
#endif

	if (DataFactory.WasInitialOnlyDataWritten())
	{
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::INITIAL_ONLY_PRESENCE_COMPONENT_ID));
	}
}

TArray<FWorkerComponentData> EntityFactory::CreateEntityComponents(USpatialActorChannel* Channel, uint32& OutBytesWritten)
{
	TArray<FWorkerComponentData> ComponentDatas = CreateMinimalEntityComponents(Channel->Actor);
	WriteUnrealComponents(ComponentDatas, Channel, OutBytesWritten);
	WriteRPCComponents(ComponentDatas, *Channel);
	WriteLBComponents(ComponentDatas, Channel->Actor);
	ComponentDatas.Add(NetDriver->InterestFactory->CreateInterestData(
		Channel->Actor, NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Channel->Actor->GetClass()), Channel->GetEntityId()));

	if (ShouldActorHaveVisibleComponent(Channel->Actor))
	{
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::VISIBLE_COMPONENT_ID));
	}

	CheckStablyNamedActorPath(ComponentDatas, Channel->Actor, Channel->GetEntityId());

	// This block of code is just for checking purposes and should be removed in the future
	// TODO: UNR-4783
#if !UE_BUILD_SHIPPING
	TArray<Worker_ComponentId> ComponentIds;
	ComponentIds.Reserve(ComponentDatas.Num());
	for (FWorkerComponentData& ComponentData : ComponentDatas)
	{
		if (ComponentIds.Contains(ComponentData.component_id))
		{
			UE_LOG(
				LogEntityFactory, Error,
				TEXT("Constructed entity components for an Unreal actor channel contained a duplicate component %i. This is unexpected and "
					 "could cause problems later on."),
				ComponentData.component_id);
		}
		ComponentIds.Add(ComponentData.component_id);
	}
#endif // !UE_BUILD_SHIPPING
	return ComponentDatas;
}

TArray<FWorkerComponentData> EntityFactory::CreateSkeletonEntityComponents(AActor* Actor)
{
	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();
	TArray<FWorkerComponentData> ComponentDatas = CreateMinimalEntityComponents(Actor);

	// LB components also contain authority delegation, giving this worker ServerAuth.
	WriteLBComponents(ComponentDatas, Actor);

	// Empty RPC components.
	Algo::Transform(SpatialRPCService::GetRPCComponents(), ComponentDatas, &ComponentFactory::CreateEmptyComponentData);
	Algo::Transform(FSpatialNetDriverRPC::GetRPCComponentIds(), ComponentDatas, &ComponentFactory::CreateEmptyComponentData);

	if (SpatialSettings->CrossServerRPCImplementation == ECrossServerRPCImplementation::RoutingWorker)
	{
		// Addition of CROSSSERVER_SENDER_ENDPOINT_COMPONENT_ID is handled in GetRPCComponentsOnEntityCreation
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CROSS_SERVER_SENDER_ACK_ENDPOINT_COMPONENT_ID));
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CROSS_SERVER_RECEIVER_ENDPOINT_COMPONENT_ID));
		ComponentDatas.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::CROSS_SERVER_RECEIVER_ACK_ENDPOINT_COMPONENT_ID));
	}

	// Skeleton entity markers.
	ComponentDatas.Emplace(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SKELETON_ENTITY_QUERY_TAG_COMPONENT_ID));
	ComponentDatas.Emplace(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SKELETON_ENTITY_POPULATION_AUTH_TAG_COMPONENT_ID));

	ComponentDatas.Add(NetDriver->InterestFactory->CreateSkeletonEntityInterest().CreateComponentData());

	return ComponentDatas;
}

void EntityFactory::CreatePopulateSkeletonComponents(USpatialActorChannel& Channel, TArray<FWorkerComponentData>& OutComponentCreates,
													 TArray<FWorkerComponentUpdate>& OutComponentUpdates, uint32& OutBytesWritten)
{
	const ComponentData* StartupActorMetadataComponent =
		NetDriver->Connection->GetCoordinator().GetComponent(Channel.GetEntityId(), UnrealMetadata::ComponentId);
	// HACK: WriteUnrealComponents needs to see
	WriteUnrealComponents(OutComponentCreates, &Channel, OutBytesWritten);
	OutComponentUpdates = { NetDriver->InterestFactory->CreateInterestUpdate(
		Channel.Actor, NetDriver->ClassInfoManager->GetOrCreateClassInfoByClass(Channel.Actor->GetClass()), Channel.GetEntityId()) };
}

TArray<FWorkerComponentData> EntityFactory::CreateTombstoneEntityComponents(AActor* Actor) const
{
	if (!ensureAlwaysMsgf(Actor->IsNetStartupActor(), TEXT("Tried to create tombstone entity components for non-net-startup Actor")))
	{
		return TArray<FWorkerComponentData>{};
	}

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
	GEngine->NetworkRemapPath(NetDriver->GetSpatialOSNetConnection(), TempPath, false /*bIsReading*/);
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
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ACTOR_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::LB_TAG_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::ROUTINGWORKER_TAG_COMPONENT_ID));

	return Components;
}

TArray<FWorkerComponentData> EntityFactory::CreatePartitionEntityComponents(const FString& PartitionName, const Worker_EntityId EntityId,
																			const InterestFactory* InterestFactory,
																			const SpatialGDK::QueryConstraint& LoadBalancingConstraint,
																			VirtualWorkerId VirtualWorker, bool bDebugContextValid)
{
	const USpatialGDKSettings* SpatialSettings = GetDefault<USpatialGDKSettings>();

	AuthorityDelegationMap DelegationMap;
	if (SpatialSettings->bRunStrategyWorker)
	{
		DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID);
		DelegationMap.Add(SpatialConstants::PARTITION_WORKER_AUTH_COMPONENT_SET_ID, EntityId);
	}
	else
	{
		DelegationMap.Add(SpatialConstants::GDK_KNOWN_ENTITY_AUTH_COMPONENT_SET_ID, EntityId);
	}

	TArray<FWorkerComponentData> Components;
	Components.Add(Position().CreateComponentData());
	Components.Add(Persistence().CreateComponentData());
	Components.Add(Metadata(FString::Format(TEXT("{0}:{1}"), { *PartitionName, VirtualWorker })).CreateComponentData());
	if (InterestFactory != nullptr)
	{
		Components.Add(InterestFactory->CreatePartitionInterest(LoadBalancingConstraint, bDebugContextValid).CreateComponentData());
	}
	Components.Add(AuthorityDelegation(DelegationMap).CreateComponentData());
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::PARTITION_SHADOW_COMPONENT_ID));
	Components.Add(ComponentFactory::CreateEmptyComponentData(SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID));

	// Worker partitions need this tag so startup handler can distinguish between them and any other partitions.
	Components.Emplace(ComponentFactory::CreateEmptyComponentData(SpatialConstants::WORKER_PARTITION_TAG_COMPONENT_ID));

	return Components;
}

} // namespace SpatialGDK
