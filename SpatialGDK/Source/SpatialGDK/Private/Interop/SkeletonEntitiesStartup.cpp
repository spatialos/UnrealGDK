#include "Interop/SkeletonEntitiesStartup.h"

#include "Algo/AllOf.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "EngineUtils.h"
#include "Interop/SkeletonEntityManifestPublisher.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Schema/SkeletonEntityManifest.h"
#include "SpatialView/ViewCoordinator.h"
#include "Utils/SpatialStatics.h"

namespace SpatialGDK
{
DEFINE_LOG_CATEGORY_STATIC(LogSpatialSkeletonEntityCreator, Log, All);

FDistributedStartupActorSkeletonEntityCreator::FDistributedStartupActorSkeletonEntityCreator(USpatialNetDriver& InNetDriver)
	: NetDriver(&InNetDriver)
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

		if (!bIsStablyNamedAndReplicated && !FUnrealObjectRef::IsUniqueActorClass(Actor->GetClass()))
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
	FCreateEntityDelegate OnCreated = [this, WeakActor = MakeWeakObjectPtr(&Actor)](const Worker_CreateEntityResponseOp& Op) {
		Worker_EntityId ActorEntityId = Op.entity_id;
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

	return SkeletonEntityFunctions::CreateSkeletonEntityForActor(Actor, *NetDriver, CreateHandler, MoveTemp(OnCreated));
}

bool FDistributedStartupActorSkeletonEntityCreator::WaitForEntities()
{
	return (Algo::AllOf(SkeletonEntitiesToDelegate,
						[View = &NetDriver->Connection->GetView()](
							const TPair<Worker_EntityId_Key, TWeakObjectPtr<AActor>>& SkeletonEntityToDelegate) -> bool {
							return View->Contains(SkeletonEntityToDelegate.Key);
						}));
}

void FDistributedStartupActorSkeletonEntityCreator::DelegateEntities()
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

		const Worker_PartitionId AuthWorkerPartition = NetDriver->VirtualWorkerTranslator->GetPartitionEntityForVirtualWorker(AuthWorkerId);

		UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("EntityId: %lld Delegating skeleton entity to VirtualWorker %d Partition %lld"),
			   CreatedSkeletonEntityId, AuthWorkerId, AuthWorkerPartition);

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
}

void FDistributedStartupActorSkeletonEntityCreator::CreateManifests()
{
	// Every worker should get a manifest to proceed through the skeleton entity flow.
	const VirtualWorkerId MaxVirtualWorkerId = static_cast<VirtualWorkerId>(NetDriver->LoadBalanceStrategy->GetMinimumRequiredWorkers());
	for (VirtualWorkerId VirtualWorker = 1; VirtualWorker <= MaxVirtualWorkerId; ++VirtualWorker)
	{
		PopulatingWorkersToEntities.FindOrAdd(VirtualWorker);
	}
	for (const auto& WorkerToEntities : PopulatingWorkersToEntities)
	{
		const VirtualWorkerId WorkerId = WorkerToEntities.Key;
		TArray<Worker_EntityId_Key> EntitiesToPopulate = WorkerToEntities.Value.Array();

		const Worker_PartitionId ServerWorkerPartitionId = NetDriver->VirtualWorkerTranslator->GetPartitionEntityForVirtualWorker(WorkerId);

		FManifestCreationHandle Handle = NetDriver->ManifestPublisher->CreateManifestForPartition(
			NetDriver->Connection->GetCoordinator(), EntitiesToPopulate, ServerWorkerPartitionId);
		if (Handle)
		{
			UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Created manifest for VirtualWorker %d WorkerPartition %lld"), WorkerId,
				   ServerWorkerPartitionId);

			Manifests.Add(Handle);
		}
	}
}

void FDistributedStartupActorSkeletonEntityCreator::CreateStrategyWorkerManifest()
{
	TArray<Worker_EntityId_Key> EntitiesToPopulate;
	for (auto& Entity : SkeletonEntitiesToDelegate)
	{
		EntitiesToPopulate.Add(Entity.Key);
	}
	FManifestCreationHandle Handle = NetDriver->ManifestPublisher->CreateManifestForPartition(
		NetDriver->Connection->GetCoordinator(), EntitiesToPopulate, SpatialConstants::INITIAL_STRATEGY_PARTITION_ENTITY_ID);
	if (Handle)
	{
		Manifests.Add(Handle);
	}
}

void FDistributedStartupActorSkeletonEntityCreator::Advance()
{
	CreateHandler.ProcessOps(NetDriver->Connection->GetWorkerMessages());

	if (Stage == EStage::WaitingForEntities)
	{
		if (WaitForEntities())
		{
			if (USpatialStatics::IsStrategyWorkerEnabled())
			{
				Stage = EStage::CreatingManifests;
			}
			else
			{
				// Proceed to the next step once all entities are in view.
				Stage = EStage::DelegatingEntities;
			}
		}
	}
	if (Stage == EStage::DelegatingEntities)
	{
		DelegateEntities();
		Stage = EStage::CreatingManifests;
		UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Finished delegating entities, moving on to manifest"));
	}

	if (Stage == EStage::CreatingManifests)
	{
		if (USpatialStatics::IsStrategyWorkerEnabled())
		{
			CreateStrategyWorkerManifest();
		}
		else
		{
			CreateManifests();
		}
		UE_LOG(LogSpatialSkeletonEntityCreator, Log, TEXT("Finished creating manifests"));

		Stage = EStage::WaitingForPopulation;
	}

	if (Stage == EStage::WaitingForPopulation)
	{
		const bool bAllManifestsFilled = Algo::AllOf(Manifests, [this](const FManifestCreationHandle& Handle) {
			return NetDriver->ManifestPublisher->HasManifestBeenProcessed(Handle);
		});

		if (bAllManifestsFilled)
		{
			Stage = EStage::Finished;
		}
	}
}

bool FDistributedStartupActorSkeletonEntityCreator::IsReady() const
{
	return Stage == EStage::Finished;
}

} // namespace SpatialGDK
