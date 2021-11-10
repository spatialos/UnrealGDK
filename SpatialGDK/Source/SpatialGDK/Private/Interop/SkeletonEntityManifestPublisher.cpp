#include "Interop/SkeletonEntityManifestPublisher.h"

#include "Algo/AllOf.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Schema/Interest.h"
#include "Schema/SkeletonEntityManifest.h"
#include "Schema/StandardLibrary.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/ViewCoordinator.h"

namespace SpatialGDK
{
static ComponentData Convert(const FWorkerComponentData& WorkerComponentData)
{
	return ComponentData(OwningComponentDataPtr(WorkerComponentData.schema_type), WorkerComponentData.component_id);
}

struct FManifestCreationData
{
	Worker_EntityId ManifestEntity = 0;
	bool bHasBeenProcessed = false;
};

FSkeletonManifestPublisher::FSkeletonManifestPublisher(const FSubView& InFilledManifestSubView)
	: FilledManifestsSubview(InFilledManifestSubView)
{
}

FManifestCreationHandle FSkeletonManifestPublisher::CreateManifestForPartition(ISpatialOSWorker& Connection,
																			   const TArray<Worker_EntityId_Key>& Entities,
																			   Worker_PartitionId Partition)
{
	FManifestCreationHandle NewHandle = MakeShared<FManifestCreationData>();

	TArray<ComponentData> SkeletonEntityManifestComponents;

	{
		const Metadata ManifestMetadata(TEXT("SkeletonEntityManifest"));
		SkeletonEntityManifestComponents.Emplace(Convert(ManifestMetadata.CreateComponentData()));
	}

	{
		FSkeletonEntityManifest Manifest;
		Manifest.EntitiesToPopulate.Append(Entities);

		SkeletonEntityManifestComponents.Emplace(Manifest.CreateComponentData());
	}

	{
		AuthorityDelegation Authority;
		Authority.Delegations.Emplace(SpatialConstants::SKELETON_ENTITY_MANIFEST_AUTH_COMPONENT_SET_ID, Partition);

		SkeletonEntityManifestComponents.Emplace(
			ComponentData(OwningComponentDataPtr(Authority.CreateComponentData().schema_type), AuthorityDelegation::ComponentId));
	}

	{
		Query ManifestQuery;
		ManifestQuery.Constraint.bSelfConstraint = true;
		ManifestQuery.ResultComponentIds.Append({ SpatialConstants::SKELETON_ENTITY_MANIFEST_COMPONENT_ID });

		// Give the server that's auth over the manifest interest in it.
		Interest ManifestAuthWorkerSelfInterest;
		ManifestAuthWorkerSelfInterest.ComponentInterestMap.Emplace(SpatialConstants::SKELETON_ENTITY_MANIFEST_AUTH_COMPONENT_SET_ID,
																	ComponentSetInterest{ { ManifestQuery } });

		SkeletonEntityManifestComponents.Emplace(
			ComponentData(OwningComponentDataPtr(ManifestAuthWorkerSelfInterest.CreateComponentData().schema_type), Interest::ComponentId));
	}

	// Required SpatialOS component.
	SkeletonEntityManifestComponents.Emplace(ComponentData(Position::ComponentId));

	Worker_RequestId RequestId = Connection.SendCreateEntityRequest(MoveTemp(SkeletonEntityManifestComponents), /*EntityId =*/{});
	CommandsHandler.AddRequest(RequestId, [NewHandle](const Worker_CreateEntityResponseOp& Op) {
		NewHandle->ManifestEntity = Op.entity_id;
	});

	InFlightManifests.Add(NewHandle);

	return NewHandle;
}

void FSkeletonManifestPublisher::Advance(ISpatialOSWorker& Connection)
{
	CommandsHandler.ProcessOps(Connection.GetWorkerMessages());
	for (auto Iter = InFlightManifests.CreateIterator(); Iter; ++Iter)
	{
		FManifestCreationData& ManifestState = *(*Iter);
		if (ManifestState.ManifestEntity == 0)
		{
			continue;
		}
		if (FilledManifestsSubview.IsEntityComplete(ManifestState.ManifestEntity))
		{
			Connection.SendDeleteEntityRequest(ManifestState.ManifestEntity);
			ManifestState.bHasBeenProcessed = true;
			Iter.RemoveCurrent();
		}
	}
}

bool FSkeletonManifestPublisher::HasManifestBeenProcessed(FManifestCreationHandle Handle) const
{
	if (!Handle.IsValid())
	{
		return true;
	}
	return Handle->bHasBeenProcessed;
}

} // namespace SpatialGDK
