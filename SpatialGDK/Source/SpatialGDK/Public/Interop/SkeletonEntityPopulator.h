#pragma once

#include "Interop/SkeletonEntities.h"

namespace SpatialGDK
{
// Utility class that processes manifests as they are assigned to the current worker,
// fleshes out the corresponding actors and update the manifest accordingly.
class FSkeletonEntityPopulator
{
public:
	explicit FSkeletonEntityPopulator(USpatialNetDriver& InNetDriver);

	void Advance(ISpatialOSWorker& Connection);
	bool HasReceivedManifests() const;
	bool IsReady() const;

private:
	ViewCoordinator& GetCoordinator();

	USpatialNetDriver* NetDriver;
	const FSubView& LocallyAuthSkeletonEntityManifestsSubview;
	const FSubView& SkeletonEntitiesRequiringPopulationSubview;

	struct ManifestProcessing
	{
		ManifestProcessing(USpatialNetDriver* InNetDriver)
			: NetDriver(InNetDriver)
		{
		}
		USpatialNetDriver* NetDriver;
		Worker_EntityId ManifestEntityId;
		FSkeletonEntityManifest Manifest;

		bool bIsFirstPopulatingEntitiesCall = true;

		ViewCoordinator& GetCoordinator();
		void Advance(FSkeletonEntityPopulator& Populator);
		void ConsiderEntityPopulation(Worker_EntityId EntityId, const EntityViewElement& Element);
		void PopulateEntity(Worker_EntityId SkeletonEntityId, AActor& SkeletonEntityStartupActor);

		enum class EStage : uint8
		{
			DiscoverAssignedSkeletonEntities,
			RefreshCompleteness,
			Finished,
		} Stage = EStage::DiscoverAssignedSkeletonEntities;
	};

	TMap<Worker_EntityId_Key, ManifestProcessing> Manifests;
	uint32 NumManifestProcessed = 0;
};

} // namespace SpatialGDK
