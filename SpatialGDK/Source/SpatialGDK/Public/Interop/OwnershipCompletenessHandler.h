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
	static FOwnershipCompletenessHandler CreateServerOwnershipHandler() { return FOwnershipCompletenessHandler(/*bInIsServer =*/true); }
	static FOwnershipCompletenessHandler CreateClientOwnershipHandler() { return FOwnershipCompletenessHandler(/*bInIsServer =*/false); }

	bool IsOwnershipComplete(Worker_EntityId EntityId, const EntityViewElement& Entity) const;
	void AddPlayerEntity(Worker_EntityId EntityId);
	void RemovePlayerEntity(Worker_EntityId EntityId);
	void AddSubView(FSubView& InSubView);

	static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);

private:
	explicit FOwnershipCompletenessHandler(const bool bInIsServer)
		: bIsServer(bInIsServer)
	{
	}

	bool bIsServer;
	TSet<Worker_EntityId_Key> PlayerOwnedEntities;
	TArray<FSubView*> SubViewsToRefresh;
};
} // namespace SpatialGDK
