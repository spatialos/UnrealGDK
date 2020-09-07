// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SubView.h"
#include "SpatialView/ConnectionHandler/AbstractConnectionHandler.h"
#include "SpatialView/Dispatcher.h"
#include "SpatialView/WorkerView.h"
#include "Templates/UniquePtr.h"

namespace SpatialGDK
{
class ViewCoordinator
{
public:
	explicit ViewCoordinator(TUniquePtr<AbstractConnectionHandler> ConnectionHandler);

	~ViewCoordinator();

	// Moveable, not copyable.
	ViewCoordinator(const ViewCoordinator&) = delete;
	ViewCoordinator(ViewCoordinator&&) = delete;
	ViewCoordinator& operator=(const ViewCoordinator&) = delete;
	ViewCoordinator& operator=(ViewCoordinator&&) = delete;

	void Advance();
	const ViewDelta& GetViewDelta() const;
	const EntityView& GetView() const;
	void FlushMessagesToSend();

	SubView& CreateSubView(const Worker_ComponentId Tag, FFilterPredicate Filter, TArray<FDispatcherRefreshCallback> DispatcherRefreshCallbacks);
	SubView& CreateUnfilteredSubView(const Worker_ComponentId Tag);
	void RefreshEntityCompleteness(const Worker_EntityId EntityId);

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

private:
	WorkerView View;
	TUniquePtr<AbstractConnectionHandler> ConnectionHandler;
	Worker_RequestId NextRequestId;
	FDispatcher Dispatcher;
	TArray<SubView> SubViews;
};

} // namespace SpatialGDK
