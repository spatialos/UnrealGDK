// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ConnectionHandler/AbstractConnectionHandler.h"
#include "SpatialView/Dispatcher.h"
#include "SpatialView/WorkerView.h"
#include "SubView.h"
#include "Templates/UniquePtr.h"

namespace SpatialGDK
{
class ViewCoordinator
{
public:
	explicit ViewCoordinator(TUniquePtr<AbstractConnectionHandler> ConnectionHandler);

	~ViewCoordinator();

	// Movable, Non-copyable.
	ViewCoordinator(const ViewCoordinator&) = delete;
	ViewCoordinator(ViewCoordinator&&) = default;
	ViewCoordinator& operator=(const ViewCoordinator&) = delete;
	ViewCoordinator& operator=(ViewCoordinator&&) = default;

	void Advance();
	const ViewDelta& GetViewDelta() const;
	const EntityView& GetView() const;
	void FlushMessagesToSend();

	// Create a subview with the specified tag, filter, and refresh callbacks.
	FSubView& CreateSubView(Worker_ComponentId Tag, const FFilterPredicate& Filter,
							const TArray<FDispatcherRefreshCallback>& DispatcherRefreshCallbacks);
	// Force a refresh of the given entity ID across all subviews. Used when local state changes which could
	// change any subview's filter's truth value for the given entity. Conceptually this can be thought of
	// as marking the entity dirty for all subviews, although the refresh is immediate.
	// Note: It would be possible to only refresh the subviews that require refreshing instead of globally refreshing.
	// This could be achieved by either having systems understand which subviews need refreshing, or by having systems
	// broadcast events which subviews subscribe to in order to trigger a refresh. This global refresh may only be a
	// temporary solution which keeps the API simple while the main systems are Actors and the load balancer.
	// In the future when there could be an unbounded number of user systems this should probably be revisited.
	void RefreshEntityCompleteness(Worker_EntityId EntityId);

	const FString& GetWorkerId() const;
	const TArray<FString>& GetWorkerAttributes() const;

	void SendAddComponent(Worker_EntityId EntityId, ComponentData Data);
	void SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update);
	void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId);
	Worker_RequestId SendReserveEntityIdsRequest(uint32 NumberOfEntityIds, TOptional<uint32> TimeoutMillis = {});
	Worker_RequestId SendCreateEntityRequest(TArray<ComponentData> EntityComponents, TOptional<Worker_EntityId> EntityId,
											 TOptional<uint32> TimeoutMillis = {});
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId, TOptional<uint32> TimeoutMillis = {});
	Worker_RequestId SendEntityQueryRequest(EntityQuery Query, TOptional<uint32> TimeoutMillis = {});
	Worker_RequestId SendEntityCommandRequest(Worker_EntityId EntityId, CommandRequest Request, TOptional<uint32> TimeoutMillis = {});
	void SendEntityCommandResponse(Worker_RequestId RequestId, CommandResponse Response);
	void SendEntityCommandFailure(Worker_RequestId RequestId, FString Message);
	void SendMetrics(SpatialMetrics Metrics);
	void SendLogMessage(Worker_LogLevel Level, const FName& LoggerName, FString Message);

	CallbackId RegisterComponentAddedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterComponentRemovedCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterComponentValueCallback(Worker_ComponentId ComponentId, FComponentValueCallback Callback);
	CallbackId RegisterAuthorityGainedCallback(Worker_ComponentId ComponentId, FEntityCallback Callback);
	CallbackId RegisterAuthorityLostCallback(Worker_ComponentId ComponentId, FEntityCallback Callback);
	CallbackId RegisterAuthorityLostTempCallback(Worker_ComponentId ComponentId, FEntityCallback Callback);
	void RemoveCallback(CallbackId Id);

	FDispatcherRefreshCallback CreateComponentExistenceRefreshCallback(
		Worker_ComponentId ComponentId,
		const FComponentChangeRefreshPredicate& RefreshPredicate = FSubView::NoComponentChangeRefreshPredicate);
	FDispatcherRefreshCallback CreateComponentChangedRefreshCallback(
		Worker_ComponentId ComponentId,
		const FComponentChangeRefreshPredicate& RefreshPredicate = FSubView::NoComponentChangeRefreshPredicate);
	FDispatcherRefreshCallback CreateAuthorityChangeRefreshCallback(
		Worker_ComponentId ComponentId,
		const FAuthorityChangeRefreshPredicate& RefreshPredicate = FSubView::NoAuthorityChangeRefreshPredicate);

private:
	WorkerView View;
	TUniquePtr<AbstractConnectionHandler> ConnectionHandler;
	Worker_RequestId NextRequestId;
	FDispatcher Dispatcher;
	TArray<TUniquePtr<FSubView>> SubViews;
};

} // namespace SpatialGDK
