#pragma once

#include "Interop/SkeletonEntities.h"

namespace SpatialGDK
{
// Utility class to publish entity manifests and track the moment the have been processed
class FSkeletonManifestPublisher
{
public:
	FSkeletonManifestPublisher(const FSubView& FilledManifestSubView);

	FManifestCreationHandle CreateManifestForPartition(ISpatialOSWorker& Connection, const TArray<Worker_EntityId_Key>& Entities,
													   Worker_PartitionId PartitionId);
	void Advance(ISpatialOSWorker& Connection);
	bool HasManifestBeenProcessed(FManifestCreationHandle) const;

protected:
	TSet<FManifestCreationHandle> InFlightManifests;
	const FSubView& FilledManifestsSubview;
	FCommandsHandler CommandsHandler;
};

} // namespace SpatialGDK
