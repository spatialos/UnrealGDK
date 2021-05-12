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
	explicit FOwnershipCompletenessHandler(bool bInIsServer)
		: bIsServer(bInIsServer)
	{
	}

	bool IsOwnershipComplete(Worker_EntityId EntityId, const EntityViewElement& Entity) const;
	void AddPlayerEntity(Worker_EntityId EntityId);
	void RemovePlayerEntity(Worker_EntityId EntityId);
	void AddSubView(FSubView& InSubView);

	static TArray<FDispatcherRefreshCallback> GetCallbacks(ViewCoordinator& Coordinator);

private:
	bool bIsServer;
	TSet<Worker_EntityId_Key> PlayerOwnedEntities;
	TArray<FSubView*> SubViewsToRefresh;
};
} // namespace SpatialGDK
