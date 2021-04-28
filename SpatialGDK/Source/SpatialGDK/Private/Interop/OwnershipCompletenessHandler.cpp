#include "Interop/OwnershipCompletenessHandler.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/ActorSystem.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Interop/InitialOnlyFilter.h"
#include "Schema/ActorSetMember.h"
#include "Schema/Restricted.h"
#include "Schema/Tombstone.h"
#include "Schema/UnrealMetadata.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/EntityView.h"
#include "SpatialView/SubView.h"
#include "SpatialView/ViewCoordinator.h"

#include "SpatialView/OpList/EntityComponentOpList.h"
#include "Tests/SpatialView/SpatialViewUtils.h"
#include "Tests/TestDefinitions.h"

namespace SpatialGDK
{
bool FOwnershipCompletenessHandler::IsOwnershipComplete(Worker_EntityId EntityId, const EntityViewElement& Entity) const
{
	const ActorOwnership Value = *Entity.Components.FindByPredicate(ComponentIdEquality{ ActorOwnership::ComponentId });

	const bool bIsPlayerOwned = Value.OwnerActorEntityId != SpatialConstants::INVALID_ENTITY_ID
								&& (Value.OwnerActorEntityId == EntityId || PlayerOwnedEntities.Contains(EntityId)
									|| PlayerOwnedEntities.Contains(Value.OwnerActorEntityId));

	const bool bShouldHaveOwnerOnlyComponents = bIsServer || bIsPlayerOwned;

	const bool bHasOwnerOnlyComponents =
		Entity.Components.ContainsByPredicate(ComponentIdEquality{ SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID });

	return bShouldHaveOwnerOnlyComponents == bHasOwnerOnlyComponents;
}

void FOwnershipCompletenessHandler::AddPlayerEntity(Worker_EntityId EntityId)
{
	PlayerOwnedEntities.Add(EntityId);

	for (FSubView* SubViewToRefresh : SubViewsToRefresh)
	{
		SubViewToRefresh->Refresh();
	}
}

void FOwnershipCompletenessHandler::RemovePlayerEntity(Worker_EntityId EntityId)
{
	PlayerOwnedEntities.Remove(EntityId);

	for (FSubView* SubViewToRefresh : SubViewsToRefresh)
	{
		SubViewToRefresh->Refresh();
	}
}

void FOwnershipCompletenessHandler::AddSubView(FSubView& InSubView)
{
	SubViewsToRefresh.Emplace(&InSubView);
}

TArray<FDispatcherRefreshCallback> FOwnershipCompletenessHandler::GetCallbacks(ViewCoordinator& Coordinator)
{
	return { Coordinator.CreateComponentChangedRefreshCallback(ActorOwnership::ComponentId),
			 Coordinator.CreateComponentExistenceRefreshCallback(SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID) };
}

} // namespace SpatialGDK
