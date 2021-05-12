#include "Interop/OwnershipCompletenessHandler.h"

#include "Schema/ActorOwnership.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/SubView.h"
#include "SpatialView/ViewCoordinator.h"

namespace SpatialGDK
{
bool FOwnershipCompletenessHandler::IsOwnershipComplete(Worker_EntityId EntityId, const EntityViewElement& Entity) const
{
	const ComponentData* OwnershipData = Entity.Components.FindByPredicate(ComponentIdEquality{ ActorOwnership::ComponentId });
	if (!ensureMsgf(OwnershipData != nullptr, TEXT("EntityId: %lld should have ActorOwnership, component ID %ld"), EntityId,
					ActorOwnership::ComponentId))
	{
		return false;
	}

	const ActorOwnership Value(*OwnershipData);

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
