#pragma once

#include "Interop/SkeletonEntities.h"

namespace SpatialGDK
{
class FDistributedStartupActorSkeletonEntityCreator
{
public:
	explicit FDistributedStartupActorSkeletonEntityCreator(USpatialNetDriver& InNetDriver);

	void CreateSkeletonEntitiesForWorld(UWorld& World);
	void Advance();
	bool IsReady() const;

private:
	Worker_EntityId CreateSkeletonEntityForActor(AActor& Actor);

	bool WaitForEntities();
	void DelegateEntities();
	void CreateManifests();
	void CreateStrategyWorkerManifest();

	USpatialNetDriver* NetDriver;
	FCreateEntityHandler CreateHandler;
	TSet<Worker_EntityId_Key> RemainingSkeletonEntities;
	TArray<TPair<Worker_EntityId_Key, TWeakObjectPtr<AActor>>> SkeletonEntitiesToDelegate;

	TMap<VirtualWorkerId, TSet<Worker_EntityId_Key>> PopulatingWorkersToEntities;

	int ManifestsCreatedCount = 0;

	bool bIsFirstTimeProcessingManifests = true;
	TArray<FManifestCreationHandle> Manifests;

	enum class EStage : uint8
	{
		Initial,
		CreatingEntities,
		WaitingForEntities,
		DelegatingEntities,
		CreatingManifests,
		WaitingForPopulation,
		Finished,
	} Stage = EStage::Initial;
};

} // namespace SpatialGDK
