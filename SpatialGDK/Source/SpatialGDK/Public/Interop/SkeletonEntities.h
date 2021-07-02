#pragma once

#include "CoreMinimal.h"

#include "Connection/SpatialWorkerConnection.h"
#include "CreateEntityHandler.h"
#include "GameFramework/Pawn.h"
#include "Schema/SkeletonEntityManifest.h"

class USpatialNetDriver;
namespace SpatialGDK
{
class FSubView;

class FDistributedStartupActorSkeletonEntityCreator
{
public:
	explicit FDistributedStartupActorSkeletonEntityCreator(USpatialNetDriver& InNetDriver);

	void CreateSkeletonEntitiesForWorld(UWorld& World);
	Worker_EntityId CreateSkeletonEntityForActor(AActor& Actor);
	void Advance();
	bool IsReady() const;

	void HackAddManifest(const Worker_EntityId EntityId, const FSkeletonEntityManifest& Manifest);

private:
	void ReadManifestFromEntity(Worker_EntityId ManifestEntityId);

	USpatialNetDriver* NetDriver;
	FCreateEntityHandler CreateHandler;
	TSet<Worker_EntityId_Key> RemainingSkeletonEntities;
	TArray<TPair<Worker_EntityId_Key, TWeakObjectPtr<AActor>>> SkeletonEntitiesToDelegate;

	TMap<VirtualWorkerId, TSet<Worker_EntityId_Key>> PopulatingWorkersToEntities;

	int ManifestsCreatedCount = 0;

	FSubView* FilledManifestsSubview;
	bool bIsFirstTimeProcessingManifests = true;
	TMap<Worker_EntityId_Key, FSkeletonEntityManifest> Manifests;

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

class FSkeletonEntityPopulator
{
public:
	explicit FSkeletonEntityPopulator(USpatialNetDriver& InNetDriver);

	void Advance();
	void ConsiderEntityPopulation(Worker_EntityId EntityId, const EntityViewElement& Element);
	void PopulateEntity(Worker_EntityId SkeletonEntityId, AActor& SkeletonEntityStartupActor);
	bool IsReady() const;

private:
	ViewCoordinator& GetCoordinator();

	USpatialNetDriver* NetDriver;
	FSubView* LocallyAuthSkeletonEntityManifestsSubview;
	FSubView* SkeletonEntitiesRequiringPopulationSubview;

	TOptional<Worker_EntityId> ManifestEntityId;
	TOptional<FSkeletonEntityManifest> Manifest;

	bool bIsFirstPopulatingEntitiesCall = true;

	enum class EStage : uint8
	{
		Initial,
		ReceivingManifest,
		DiscoverAssignedSkeletonEntities,
		RefreshCompleteness,
		Finished,
	} Stage = EStage::Initial;
};

} // namespace SpatialGDK
