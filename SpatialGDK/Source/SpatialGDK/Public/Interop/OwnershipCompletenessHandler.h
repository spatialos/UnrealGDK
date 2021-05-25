#pragma once

#include "Misc/Optional.h"

#include "SpatialCommonTypes.h"
#include "SpatialView/SubView.h"

class USpatialNetDriver;

namespace SpatialGDK
{
class ViewCoordinator;

class FOwnershipCompletenessHandler
{
public:
	enum class EOwnershipCompletenessStrategy
	{
		AlwaysHasOwnerComponents,
		RequiresPlayerOwnership,
	};

	static FOwnershipCompletenessHandler CreateServerOwnershipHandler()
	{
		return FOwnershipCompletenessHandler(EOwnershipCompletenessStrategy::AlwaysHasOwnerComponents);
	}
	static FOwnershipCompletenessHandler CreateClientOwnershipHandler()
	{
		return FOwnershipCompletenessHandler(EOwnershipCompletenessStrategy::RequiresPlayerOwnership);
	}

	bool IsOwnershipComplete(Worker_EntityId EntityId, const EntityViewElement& Entity) const;
	void AddPlayerEntity(Worker_EntityId EntityId);
	void TryRemovePlayerEntity(Worker_EntityId EntityId);
	void AddSubView(FSubView& InSubView);

	static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);

private:
	bool ShouldHaveOwnerOnlyComponents(Worker_EntityId EntityId, const EntityViewElement& Entity) const;

	explicit FOwnershipCompletenessHandler(const EOwnershipCompletenessStrategy InStrategy)
		: Strategy(InStrategy)
	{
	}

	EOwnershipCompletenessStrategy Strategy;
	TSet<Worker_EntityId_Key> PlayerOwnedEntities;
	TArray<FSubView*> SubViewsToRefresh;
};
} // namespace SpatialGDK
