#include "Interop/SkeletonEntities.h"

#include "Algo/AllOf.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriverRPC.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "EngineUtils.h"
#include "Interop/ActorSystem.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/SpatialSender.h"
#include "Interop/WellKnownEntitySystem.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/SkeletonEntityManifest.h"
#include "SpatialView/ViewCoordinator.h"
#include "Utils/ComponentFactory.h"
#include "Utils/EntityFactory.h"
#include "Utils/EntityPool.h"
#include "Utils/InterestFactory.h"

namespace SpatialGDK
{
DEFINE_LOG_CATEGORY_STATIC(LogSpatialSkeletonEntityCreator, Log, All);

static ComponentData Convert(const FWorkerComponentData& WorkerComponentData)
{
	return ComponentData(OwningComponentDataPtr(WorkerComponentData.schema_type), WorkerComponentData.component_id);
};

static void ForEachCompleteEntity(bool& bIsFirstCall, const FSubView& SubView, const TFunction<void(Worker_EntityId)> Handler)
{
	if (bIsFirstCall)
	{
		bIsFirstCall = false;
		for (const Worker_EntityId EntityId : SubView.GetCompleteEntities())
		{
			Handler(EntityId);
		}
	}
	else
	{
		for (const EntityDelta& Delta : SubView.GetViewDelta().EntityDeltas)
		{
			checkf(Delta.Type != EntityDelta::REMOVE,
				   TEXT("Skeleton entity manifest must never be removed, but was removed from entity %lld"), Delta.EntityId)

				if (Delta.Type == EntityDelta::ADD || Delta.Type == EntityDelta::UPDATE || Delta.Type == EntityDelta::TEMPORARILY_REMOVED)
			{
				const Worker_EntityId EntityId = Delta.EntityId;
				Handler(EntityId);
			}
		}
	}
};

static bool IsManifestFilled(const Worker_EntityId, const EntityViewElement& Entity)
{
	const ComponentData* ManifestComponentDataPtr =
		Entity.Components.FindByPredicate(ComponentIdEquality{ FSkeletonEntityManifest::ComponentId });
	if (ensure(ManifestComponentDataPtr != nullptr))
	{
		const FSkeletonEntityManifest Manifest(*ManifestComponentDataPtr);
		return Manifest.PopulatedEntities.Num() == Manifest.EntitiesToPopulate.Num();
	}
	return false;
}

FDistributedStartupActorSkeletonEntityCreator::FDistributedStartupActorSkeletonEntityCreator(USpatialNetDriver& InNetDriver)
	: NetDriver(&InNetDriver)
	, FilledManifestsSubview(&NetDriver->Connection->GetCoordinator().CreateSubView(
		  FSkeletonEntityManifest::ComponentId, &IsManifestFilled,
		  { NetDriver->Connection->GetCoordinator().CreateComponentChangedRefreshCallback(FSkeletonEntityManifest::ComponentId) }))
{
}

void FDistributedStartupActorSkeletonEntityCreator::CreateSkeletonEntitiesForWorld(UWorld& World)
{
	ensure(Stage == EStage::Initial);

	UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Creating skeleton entities for world %s"), *World.GetName())

	Stage = EStage::CreatingEntities;

	TSet<Worker_EntityId_Key> SkeletonEntityIds;

	for (AActor* Actor : TActorRange<AActor>(&World))
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		if (!Actor->GetIsReplicated())
		{
			continue;
		}

		const bool bIsStartupActor = Actor->IsNetStartupActor();
		const bool bIsStablyNamedAndReplicated = Actor->IsNameStableForNetworking();

		checkf(bIsStartupActor == bIsStablyNamedAndReplicated,
			   TEXT("Actor %s is startup %s stably named %s replicated; startup should be the same as Stably Named."), *Actor->GetName(),
			   bIsStartupActor ? TEXT("True") : TEXT("False"), Actor->IsNameStableForNetworking() ? TEXT("True") : TEXT("False"));

		if (!bIsStablyNamedAndReplicated)
		{
			continue;
		}

		const Worker_EntityId SkeletonEntityId = CreateSkeletonEntityForActor(*Actor);

		UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("EntityId: %lld Creating skeleton entity for %s"), SkeletonEntityId,
			   *Actor->GetName());

		RemainingSkeletonEntities.Emplace(SkeletonEntityId);
		SkeletonEntityIds.Emplace(SkeletonEntityId);
	}

	if (RemainingSkeletonEntities.Num() == 0)
	{
		UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("No skeleton entities must be created for %s"), *World.GetName())

		Stage = EStage::DelegatingEntities;
	}
}

Worker_EntityId FDistributedStartupActorSkeletonEntityCreator::CreateSkeletonEntityForActor(AActor& Actor)
{
	const Worker_EntityId ActorEntityId = NetDriver->PackageMap->AllocateEntityId();

	EntityFactory Factory(NetDriver, NetDriver->PackageMap, NetDriver->ClassInfoManager, NetDriver->RPCService.Get());

	TArray<FWorkerComponentData> EntityComponents = Factory.CreateSkeletonEntityComponents(&Actor);

	// LB components also contain authority delegation, giving this worker ServerAuth.
	Factory.WriteLBComponents(EntityComponents, &Actor);

	// RPC components.
	Algo::Transform(SpatialRPCService::GetRPCComponents(), EntityComponents, &ComponentFactory::CreateEmptyComponentData);
	Algo::Transform(FSpatialNetDriverRPC::GetRPCComponentIds(), EntityComponents, &ComponentFactory::CreateEmptyComponentData);

	// Skeleton entity markers		.
	EntityComponents.Emplace(ComponentFactory::CreateEmptyComponentData(SpatialConstants::SKELETON_ENTITY_QUERY_TAG_COMPONENT_ID));
	EntityComponents.Emplace(
		ComponentFactory::CreateEmptyComponentData(SpatialConstants::SKELETON_ENTITY_POPULATION_AUTH_TAG_COMPONENT_ID));

	Interest SkeletonEntityInterest;
	NetDriver->InterestFactory->AddServerSelfInterest(SkeletonEntityInterest);
	EntityComponents.Add(SkeletonEntityInterest.CreateComponentData());

	TArray<ComponentData> SkeletonEntityComponentDatas;
	Algo::Transform(EntityComponents, SkeletonEntityComponentDatas, &Convert);

	ViewCoordinator& Coordinator = NetDriver->Connection->GetCoordinator();
	const Worker_RequestId CreateEntityRequestId =
		Coordinator.SendCreateEntityRequest(MoveTemp(SkeletonEntityComponentDatas), ActorEntityId);

	FCreateEntityDelegate OnCreated = [this, ActorEntityId, WeakActor = MakeWeakObjectPtr(&Actor)](const Worker_CreateEntityResponseOp&) {
		RemainingSkeletonEntities.Remove(ActorEntityId);
		if (WeakActor.IsValid())
		{
			SkeletonEntitiesToDelegate.Emplace(ActorEntityId, WeakActor);
			UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("EntityId: %lld Created skeleton entity"), ActorEntityId);
		}
		if (RemainingSkeletonEntities.Num() == 0)
		{
			Stage = EStage::WaitingForEntities;
		}
	};
	CreateHandler.AddRequest(CreateEntityRequestId, MoveTemp(OnCreated));

	return ActorEntityId;
}

void FDistributedStartupActorSkeletonEntityCreator::Advance()
{
	CreateHandler.ProcessOps(NetDriver->Connection->GetWorkerMessages());

	if (Stage == EStage::WaitingForEntities)
	{
		if (Algo::AllOf(SkeletonEntitiesToDelegate,
						[View = &NetDriver->Connection->GetView()](
							const TPair<Worker_EntityId_Key, TWeakObjectPtr<AActor>>& SkeletonEntityToDelegate) -> bool {
							return View->Contains(SkeletonEntityToDelegate.Key);
						}))
		{
			// Proceed to the next step once all entities are in view.
			Stage = EStage::DelegatingEntities;
		}
	}
	if (Stage == EStage::DelegatingEntities)
	{
		for (const TPair<Worker_EntityId_Key, TWeakObjectPtr<AActor>>& SkeletonEntityAndActor : SkeletonEntitiesToDelegate)
		{
			if (!SkeletonEntityAndActor.Value.IsValid())
			{
				continue;
			}

			const Worker_EntityId CreatedSkeletonEntityId = SkeletonEntityAndActor.Key;
			AActor& SkeletonEntityActor = *SkeletonEntityAndActor.Value;

			const VirtualWorkerId AuthWorkerId = NetDriver->LoadBalanceStrategy->WhoShouldHaveAuthority(SkeletonEntityActor);

			const Worker_PartitionId AuthWorkerPartition =
				NetDriver->VirtualWorkerTranslator->GetPartitionEntityForVirtualWorker(AuthWorkerId);

			UE_LOG(LogSpatialSkeletonEntityCreator, Log,
				   TEXT("EntityId: %lld Delegating skeleton entity to VirtualWorker %d Partition %lld"), CreatedSkeletonEntityId,
				   AuthWorkerId, AuthWorkerPartition);

			AuthorityDelegation AuthorityDelegationComponent;
			AuthorityDelegationComponent.Delegations.Emplace(SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID, AuthWorkerPartition);

			PopulatingWorkersToEntities.FindOrAdd(AuthWorkerId).Emplace(CreatedSkeletonEntityId);

			NetDriver->Connection->GetCoordinator().SendComponentUpdate(
				CreatedSkeletonEntityId,
				ComponentUpdate(OwningComponentUpdatePtr(AuthorityDelegationComponent.CreateAuthorityDelegationUpdate().schema_type),
								AuthorityDelegation::ComponentId),
				{});
		}

		SkeletonEntitiesToDelegate.Empty();

		UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Finished delegating entities, moving on to manifest"));

		Stage = EStage::CreatingManifests;
	}

	if (Stage == EStage::CreatingManifests)
	{
		// Every worker should get a manifest to proceed through the skeleton entity flow.
		for (const auto& VirtualWorker : NetDriver->WellKnownEntitySystem->GetVirtualWorkerTranslationManager()->GetVirtualWorkerMapping())
		{
			PopulatingWorkersToEntities.FindOrAdd(VirtualWorker.Key);
		}
		for (const auto& WorkerToEntities : PopulatingWorkersToEntities)
		{
			const VirtualWorkerId WorkerId = WorkerToEntities.Key;
			const TSet<Worker_EntityId_Key>& EntitiesToPopulate = WorkerToEntities.Value;

			TArray<ComponentData> SkeletonEntityManifestComponents;

			{
				const Metadata ManifestMetadata(TEXT("SkeletonEntityManifest"));

				SkeletonEntityManifestComponents.Emplace(Convert(ManifestMetadata.CreateComponentData()));
			}

			{
				FSkeletonEntityManifest Manifest;
				Manifest.EntitiesToPopulate = EntitiesToPopulate;

				SkeletonEntityManifestComponents.Emplace(Manifest.CreateComponentData());
			}

			const Worker_PartitionId ServerWorkerPartitionId =
				NetDriver->VirtualWorkerTranslator->GetPartitionEntityForVirtualWorker(WorkerId);

			{
				AuthorityDelegation Authority;
				Authority.Delegations.Emplace(SpatialConstants::SKELETON_ENTITY_MANIFEST_AUTH_COMPONENT_SET_ID, ServerWorkerPartitionId);

				SkeletonEntityManifestComponents.Emplace(
					ComponentData(OwningComponentDataPtr(Authority.CreateComponentData().schema_type), AuthorityDelegation::ComponentId));
			}

			{
				Query ManifestQuery;
				ManifestQuery.Constraint.bSelfConstraint = true;
				ManifestQuery.ResultComponentIds.Append({ SpatialConstants::SKELETON_ENTITY_MANIFEST_COMPONENT_ID });

				// Give the server that's auth over the manifest interest in it.
				Interest ManifestAuthWorkerSelfInterest;
				ManifestAuthWorkerSelfInterest.ComponentInterestMap.Emplace(
					SpatialConstants::SKELETON_ENTITY_MANIFEST_AUTH_COMPONENT_SET_ID, ComponentSetInterest{ { ManifestQuery } });

				SkeletonEntityManifestComponents.Emplace(ComponentData(
					OwningComponentDataPtr(ManifestAuthWorkerSelfInterest.CreateComponentData().schema_type), Interest::ComponentId));
			}

			// Required SpatialOS component.
			SkeletonEntityManifestComponents.Emplace(ComponentData(Position::ComponentId));

			ViewCoordinator& Coordinator = NetDriver->Connection->GetCoordinator();
			Coordinator.SendCreateEntityRequest(MoveTemp(SkeletonEntityManifestComponents), /*EntityId =*/{});

			UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Created manifest for VirtualWorker %d WorkerPartition %lld"), WorkerId,
				   ServerWorkerPartitionId);
		}

		ManifestsCreatedCount = PopulatingWorkersToEntities.Num();

		UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Finished creating manifests"));
		Stage = EStage::WaitingForPopulation;
	}

	if (Stage == EStage::WaitingForPopulation)
	{
		const bool bAllManifestsFilled = FilledManifestsSubview->GetCompleteEntities().Num() == ManifestsCreatedCount;

		if (bAllManifestsFilled)
		{
			Stage = EStage::Finished;
		}
	}
}

void FDistributedStartupActorSkeletonEntityCreator::ReadManifestFromEntity(const Worker_EntityId ManifestEntityId)
{
	const EntityViewElement* Element = FilledManifestsSubview->GetView().Find(ManifestEntityId);

	check(Element != nullptr);

	const ComponentData* ManifestComponent =
		Element->Components.FindByPredicate(ComponentIdEquality{ FSkeletonEntityManifest::ComponentId });

	checkf(ManifestComponent != nullptr, TEXT("Skeleton entity manifest must never be removed, but was removed from entity %lld"),
		   ManifestEntityId);

	Manifests.Add(ManifestEntityId, FSkeletonEntityManifest(*ManifestComponent));
};

bool FDistributedStartupActorSkeletonEntityCreator::IsReady() const
{
	return Stage == EStage::Finished;
}

void FDistributedStartupActorSkeletonEntityCreator::HackAddManifest(const Worker_EntityId EntityId, const FSkeletonEntityManifest& Manifest)
{
	ensure(Stage == EStage::WaitingForPopulation);

	UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Adding manifest with %d entities to %lld as a hack..."),
		   Manifest.PopulatedEntities.Num(), EntityId);

	Manifests.Add(EntityId, Manifest);
}

FSkeletonEntityPopulator::FSkeletonEntityPopulator(USpatialNetDriver& InNetDriver)
	: NetDriver(&InNetDriver)
	, LocallyAuthSkeletonEntityManifestsSubview(&InNetDriver.Connection->GetCoordinator().CreateSubView(
		  SpatialConstants::SKELETON_ENTITY_MANIFEST_COMPONENT_ID,
		  [](const Worker_EntityId EntityId, const EntityViewElement& Entity) {
			  return Entity.Authority.Contains(SpatialConstants::SKELETON_ENTITY_MANIFEST_AUTH_COMPONENT_SET_ID);
		  },
		  { GetCoordinator().CreateAuthorityChangeRefreshCallback(SpatialConstants::SKELETON_ENTITY_MANIFEST_AUTH_COMPONENT_SET_ID) }))
	, SkeletonEntitiesRequiringPopulationSubview(&InNetDriver.Connection->GetCoordinator().CreateSubView(
		  SpatialConstants::SKELETON_ENTITY_POPULATION_AUTH_TAG_COMPONENT_ID, FSubView::NoFilter, FSubView::NoDispatcherCallbacks))
{
	Stage = EStage::ReceivingManifest;
}

void FSkeletonEntityPopulator::Advance()
{
	if (Stage == EStage::ReceivingManifest)
	{
		const TArray<Worker_EntityId>& CompleteManifestEntities = LocallyAuthSkeletonEntityManifestsSubview->GetCompleteEntities();
		if (CompleteManifestEntities.Num() >= 1)
		{
			ensureMsgf(CompleteManifestEntities.Num() == 1,
					   TEXT("Skeleton entity populator only expects a single manifest to be visible, got %d instead!"),
					   CompleteManifestEntities.Num());
			ManifestEntityId = CompleteManifestEntities[0];
			const ComponentData* ManifestDataPtr = GetCoordinator().GetComponent(*ManifestEntityId, FSkeletonEntityManifest::ComponentId);
			Manifest = FSkeletonEntityManifest(*ManifestDataPtr);
			Stage = EStage::DiscoverAssignedSkeletonEntities;
		}
	}

	if (Stage == EStage::DiscoverAssignedSkeletonEntities)
	{
		ForEachCompleteEntity(bIsFirstPopulatingEntitiesCall, *SkeletonEntitiesRequiringPopulationSubview,
							  [this](const Worker_EntityId EntityId) {
								  const EntityViewElement* EntityPtr = NetDriver->Connection->GetCoordinator().GetView().Find(EntityId);

								  check(EntityPtr != nullptr);

								  ConsiderEntityPopulation(EntityId, *EntityPtr);
							  });

		GetCoordinator().SendComponentUpdate(*ManifestEntityId, Manifest->CreateComponentUpdate(), {});

		GetCoordinator().RefreshEntityCompleteness(*ManifestEntityId);

		if (Manifest->PopulatedEntities.Num() == Manifest->EntitiesToPopulate.Num())
		{
			UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("All %d entities populated"), Manifest->PopulatedEntities.Num());
			Stage = EStage::RefreshCompleteness;
		}
	}

	if (Stage == EStage::RefreshCompleteness)
	{
		// All locally populated entities need entity refreshes as this worker won't receive
		// deltas for all the components it has added, and these can impact entity completeness.
		for (const Worker_EntityId EntityId : Manifest->PopulatedEntities)
		{
			GetCoordinator().RefreshEntityCompleteness(EntityId);
		}

		Stage = EStage::Finished;
	}
}

void FSkeletonEntityPopulator::ConsiderEntityPopulation(const Worker_EntityId EntityId, const EntityViewElement& Element)
{
	const ComponentData* UnrealMetadataComponent = Element.Components.FindByPredicate(ComponentIdEquality{ UnrealMetadata::ComponentId });

	if (!ensure(UnrealMetadataComponent != nullptr))
	{
		return;
	}

	if (!ensureMsgf(Manifest->EntitiesToPopulate.Contains(EntityId),
					TEXT("Skeleton entity %lld seen on worker, but doesn't exist in the manifest!"), EntityId))
	{
		return;
	}

	const UnrealMetadata ActorMetadata(UnrealMetadataComponent->GetUnderlying());

	if (ensure(ActorMetadata.bNetStartup.IsSet() && ActorMetadata.bNetStartup.GetValue() && ActorMetadata.StablyNamedRef.IsSet()))
	{
		const FUnrealObjectRef& ActorRef = ActorMetadata.StablyNamedRef.GetValue();
		bool bUnresolved = false;
		UObject* Object = FUnrealObjectRef::ToObjectPtr(ActorRef, NetDriver->PackageMap, bUnresolved);
		if (!ensure(!bUnresolved))
		{
			return;
		}

		AActor* StartupActor = Cast<AActor>(Object);
		PopulateEntity(EntityId, *StartupActor);

		Manifest->PopulatedEntities.Emplace(EntityId);
	}
}

void FSkeletonEntityPopulator::PopulateEntity(Worker_EntityId SkeletonEntityId, AActor& SkeletonEntityStartupActor)
{
	NetDriver->PackageMap->ResolveEntityActorAndSubobjects(SkeletonEntityId, &SkeletonEntityStartupActor);

	USpatialActorChannel* Channel = ActorSystem::SetUpActorChannel(NetDriver, &SkeletonEntityStartupActor, SkeletonEntityId);

	if (ensure(IsValid(Channel)))
	{
		// Mark this channel as creating new entity as we need to remember to populate it
		// with all components when we decide to replicate it for the first time.
		Channel->bCreatingNewEntity = true;
	}
}

bool FSkeletonEntityPopulator::IsReady() const
{
	return Stage == EStage::Finished;
}

ViewCoordinator& FSkeletonEntityPopulator::GetCoordinator()
{
	return NetDriver->Connection->GetCoordinator();
}
} // namespace SpatialGDK
