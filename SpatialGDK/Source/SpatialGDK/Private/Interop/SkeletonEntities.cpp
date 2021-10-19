#include "Interop/SkeletonEntities.h"

#include "Algo/AllOf.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialNetDriverRPC.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Schema/SkeletonEntityManifest.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/ViewCoordinator.h"
#include "Utils/ComponentFactory.h"
#include "Utils/EntityFactory.h"

DEFINE_LOG_CATEGORY(LogSpatialSkeletonEntityCreator)

namespace SpatialGDK
{
static ComponentData Convert(const FWorkerComponentData& WorkerComponentData)
{
	return ComponentData(OwningComponentDataPtr(WorkerComponentData.schema_type), WorkerComponentData.component_id);
}

bool SkeletonEntityFunctions::IsCompleteSkeleton(const EntityViewElement& Entity)
{
	bool bHasSkeletonTag = false;
	bool bHasPopulatedTag = false;
	for (const ComponentData& Component : Entity.Components)
	{
		if (Component.GetComponentId() == SpatialConstants::SKELETON_ENTITY_QUERY_TAG_COMPONENT_ID)
		{
			bHasSkeletonTag = true;
		}
		if (Component.GetComponentId() == SpatialConstants::SKELETON_ENTITY_POPULATION_FINISHED_TAG_COMPONENT_ID)
		{
			bHasPopulatedTag = true;
		}
	}

	if (bHasSkeletonTag)
	{
		// Skeletons must have a populated tag for us to consider them complete.
		if (bHasPopulatedTag)
		{
			return true;
		}
		UE_LOG(LogTemp, Log, TEXT("Entity missing populated tag"));
		return false;
	}
	// Non-skeleton entities are assumed to be complete actors by default.
	return true;
}

TArray<FDispatcherRefreshCallback> SkeletonEntityFunctions::GetSkeletonEntityRefreshCallbacks(ViewCoordinator& Coordinator)
{
	return {
		Coordinator.CreateComponentExistenceRefreshCallback(SpatialConstants::SKELETON_ENTITY_QUERY_TAG_COMPONENT_ID),
		Coordinator.CreateComponentExistenceRefreshCallback(SpatialConstants::SKELETON_ENTITY_POPULATION_FINISHED_TAG_COMPONENT_ID),
	};
}

Worker_EntityId SkeletonEntityFunctions::CreateSkeletonEntityForActor(AActor& Actor, USpatialNetDriver& NetDriver,
																	  FCreateEntityHandler& CreateHandler, FCreateEntityDelegate OnCreated)
{
	const Worker_EntityId ActorEntityId = NetDriver.PackageMap->AllocateEntityId();

	EntityFactory Factory(&NetDriver, NetDriver.PackageMap, NetDriver.ClassInfoManager, NetDriver.RPCService.Get());

	const TArray<FWorkerComponentData> EntityComponents = Factory.CreateSkeletonEntityComponents(&Actor);

	TArray<ComponentData> SkeletonEntityComponentData;
	Algo::Transform(EntityComponents, SkeletonEntityComponentData, &Convert);

	ViewCoordinator& Coordinator = NetDriver.Connection->GetCoordinator();
	const Worker_RequestId CreateEntityRequestId =
		Coordinator.SendCreateEntityRequest(MoveTemp(SkeletonEntityComponentData), ActorEntityId);

	CreateHandler.AddRequest(CreateEntityRequestId, MoveTemp(OnCreated));

	return ActorEntityId;
}

static bool IsManifestFilled(const Worker_EntityId, const EntityViewElement& Entity)
{
	const ComponentData* ManifestComponentDataPtr =
		Entity.Components.FindByPredicate(ComponentIdEquality{ FSkeletonEntityManifest::ComponentId });
	if (ensure(ManifestComponentDataPtr != nullptr))
	{
		const FSkeletonEntityManifest Manifest(*ManifestComponentDataPtr);
		return Manifest.bAckedManifest && Manifest.PopulatedEntities.Num() == Manifest.EntitiesToPopulate.Num();
	}
	return false;
}

const FSubView& SkeletonEntityFunctions::CreateFilledManifestSubView(ViewCoordinator& Coordinator)
{
	return Coordinator.CreateSubView(FSkeletonEntityManifest::ComponentId, &IsManifestFilled,
									 { Coordinator.CreateComponentChangedRefreshCallback(FSkeletonEntityManifest::ComponentId) });
}

const FSubView& SkeletonEntityFunctions::CreateLocallyAuthManifestSubView(ViewCoordinator& Coordinator)
{
	return Coordinator.CreateSubView(
		SpatialConstants::SKELETON_ENTITY_MANIFEST_COMPONENT_ID,
		[](const Worker_EntityId EntityId, const SpatialGDK::EntityViewElement& Entity) {
			return Entity.Authority.Contains(SpatialConstants::SKELETON_ENTITY_MANIFEST_AUTH_COMPONENT_SET_ID);
		},
		{ Coordinator.CreateAuthorityChangeRefreshCallback(SpatialConstants::SKELETON_ENTITY_MANIFEST_AUTH_COMPONENT_SET_ID) });
}

} // namespace SpatialGDK
