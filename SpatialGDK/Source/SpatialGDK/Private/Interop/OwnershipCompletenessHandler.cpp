#include "Interop/OwnershipCompletenessHandler.h"

#include "Schema/ActorOwnership.h"
#include "SpatialView/EntityComponentTypes.h"
#include "SpatialView/SubView.h"
#include "SpatialView/ViewCoordinator.h"

namespace SpatialGDK
{
bool FOwnershipCompletenessHandler::IsOwnershipComplete(Worker_EntityId EntityId, const EntityViewElement& Entity) const
{
	const bool bShouldHaveOwnerOnlyComponents = ShouldHaveOwnerOnlyComponents(EntityId, Entity);

	const bool bHasOwnerOnlyComponents =
		Entity.Components.ContainsByPredicate(ComponentIdEquality{ SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID });

	return bShouldHaveOwnerOnlyComponents == bHasOwnerOnlyComponents;
}

bool FOwnershipCompletenessHandler::ShouldHaveOwnerOnlyComponents(Worker_EntityId EntityId, const EntityViewElement& Entity) const
{
	switch (Strategy)
	{
	case EOwnershipCompletenessStrategy::AlwaysHasOwnerComponents:
		return true;
	case EOwnershipCompletenessStrategy::RequiresPlayerOwnership:
	{
		const ComponentData* OwnershipData = Entity.Components.FindByPredicate(ComponentIdEquality{ ActorOwnership::ComponentId });
		if (!ensureMsgf(OwnershipData != nullptr, TEXT("Entity should have ActorOwnership, EntityId: %lld component ID %ld"), EntityId,
						ActorOwnership::ComponentId))
		{
			return false;
		}
		const ActorOwnership Value(*OwnershipData);
		const bool bIsPlayerOwned = Value.OwnerActorEntityId != SpatialConstants::INVALID_ENTITY_ID
									&& (Value.OwnerActorEntityId == EntityId || PlayerOwnedEntities.Contains(EntityId)
										|| PlayerOwnedEntities.Contains(Value.OwnerActorEntityId));
		return bIsPlayerOwned;
	}
	default:
		checkNoEntry();
	}
	return false;
}

void FOwnershipCompletenessHandler::AddPlayerEntity(Worker_EntityId EntityId)
{
	bool bIsAlreadyInSet = false;
	PlayerOwnedEntities.Add(EntityId, &bIsAlreadyInSet);
	if (!bIsAlreadyInSet)
	{
		bRequiresRefresh = true;
	}
}

void FOwnershipCompletenessHandler::TryRemovePlayerEntity(Worker_EntityId EntityId)
{
	int32 CountRemoved = PlayerOwnedEntities.Remove(EntityId);
	if (CountRemoved > 0)
	{
		bRequiresRefresh = true;
	}
}

void FOwnershipCompletenessHandler::AddSubView(FSubView& InSubView)
{
	SubViewsToRefresh.Emplace(&InSubView);
}

void FOwnershipCompletenessHandler::Advance()
{
	if (bRequiresRefresh)
	{
		bRequiresRefresh = false;

		for (FSubView* SubViewToRefresh : SubViewsToRefresh)
		{
			SubViewToRefresh->Refresh();
		}
	}
}

TArray<FDispatcherRefreshCallback> FOwnershipCompletenessHandler::GetCallbacks(ViewCoordinator& Coordinator)
{
	return { Coordinator.CreateComponentChangedRefreshCallback(ActorOwnership::ComponentId),
			 Coordinator.CreateComponentExistenceRefreshCallback(SpatialConstants::ACTOR_OWNER_ONLY_DATA_TAG_COMPONENT_ID) };
}
} // namespace SpatialGDK
