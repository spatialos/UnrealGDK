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

struct FDebugSkelEntityData
{
	Worker_EntityId EntityId;
	VirtualWorkerId WorkerId;
};

class FDistributedStartupActorSkeletonEntityCreator
{
public:
	explicit FDistributedStartupActorSkeletonEntityCreator(USpatialNetDriver& InNetDriver);

	Worker_EntityId CreateSkeletonEntityForActor(AActor& Actor);
	void CreateSkeletonEntitiesForLevel(UWorld& World);
	void Advance();
	bool IsReady() const;

	void HackAddManifest(const Worker_EntityId EntityId, const FSkeletonEntityManifest& Manifest);

private:
	USpatialNetDriver* NetDriver;
	CreateEntityHandler CreateHandler;
	TSet<Worker_EntityId_Key> RemainingSkeletonEntities;
	TArray<TPair<Worker_EntityId_Key, TWeakObjectPtr<AActor>>> SkeletonEntitiesToDelegate;
	TSet<Worker_EntityId_Key> CreatedSkeletonEntities;

	TMap<Worker_EntityId_Key, VirtualWorkerId> EntitiesToPopulatingWorkers;

	int ManifestsCreatedCount = 0;

	FSubView* ManifestSubView;
	bool bIsFirstTimeProcessingManifests = true;
	TMap<Worker_EntityId_Key, FSkeletonEntityManifest> Manifests;

	enum class EStage : uint8
	{
		Initial,
		CreatingEntities,
		DelegatingEntities,
		SigningManifest,
		WaitingForPopulation,
		FinalizingSkeletons,
		Finished,
	} Stage = EStage::Initial;
};

class FSkeletonEntityPopulator
{
public:
	explicit FSkeletonEntityPopulator(USpatialNetDriver& InNetDriver);
	explicit FSkeletonEntityPopulator(USpatialNetDriver& InNetDriver,
									  TFunction<void(Worker_EntityId, FSkeletonEntityManifest)> InOnManifestUpdated)
		: FSkeletonEntityPopulator(InNetDriver)
	{
		OnManifestUpdated = InOnManifestUpdated;
	}
	void ConsiderEntityPopulation(Worker_EntityId EntityId, const EntityViewElement& Element);
	void Advance();
	void PopulateEntity(Worker_EntityId SkeletonEntityId, AActor& SkeletonEntityStartupActor);
	bool IsReady() const;

private:
	ViewCoordinator& GetCoordinator();

	USpatialNetDriver* NetDriver;
	FSubView* SubView;

	TFunction<void(Worker_EntityId, FSkeletonEntityManifest)> OnManifestUpdated;

	TSet<Worker_EntityId_Key> EntitiesFleshedOut;

	TOptional<Worker_EntityId> ManifestEntityId;
	TOptional<FSkeletonEntityManifest> Manifest;

	bool bStartedPopulatingEntities = false;

	enum class EStage : uint8
	{
		Initial,
		ReceivingManifest,
		ReceivingSkeletonEntities,
		PopulatingEntities,
		SigningManifest,
		Finished,
	} Stage = EStage::Initial;
};

} // namespace SpatialGDK
