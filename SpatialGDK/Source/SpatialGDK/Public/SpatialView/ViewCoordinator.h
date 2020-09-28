// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ConnectionHandler/AbstractConnectionHandler.h"
#include "SpatialView/Dispatcher.h"
#include "SpatialView/WorkerView.h"
#include "Templates/UniquePtr.h"

namespace SpatialGDK
{
class SpatialEventTracer;

class ViewCoordinator
{
public:
	explicit ViewCoordinator(TUniquePtr<AbstractConnectionHandler> ConnectionHandler, TSharedPtr<SpatialEventTracer> EventTracer);

	~ViewCoordinator();

	// Moveable, not copyable.
	ViewCoordinator(const ViewCoordinator&) = delete;
	ViewCoordinator(ViewCoordinator&&) = default;
	ViewCoordinator& operator=(const ViewCoordinator&) = delete;
	ViewCoordinator& operator=(ViewCoordinator&&) = default;

	void Advance();
	const ViewDelta& GetViewDelta();
	const EntityView& GetView();
	void FlushMessagesToSend();

	const FString& GetWorkerId() const;
	const TArray<FString>& GetWorkerAttributes() const;

	void SendAddComponent(Worker_EntityId EntityId, ComponentData Data, const TOptional<worker::c::Trace_SpanId>& SpanId);
	void SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update, const TOptional<worker::c::Trace_SpanId>& SpanId);
	void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const TOptional<worker::c::Trace_SpanId>& SpanId);
	Worker_RequestId SendReserveEntityIdsRequest(uint32 NumberOfEntityIds, TOptional<uint32> TimeoutMillis = {});
	Worker_RequestId SendCreateEntityRequest(TArray<ComponentData> EntityComponents, TOptional<Worker_EntityId> EntityId,
											 TOptional<uint32> TimeoutMillis = {}, const TOptional<worker::c::Trace_SpanId>& SpanId = {});
	Worker_RequestId SendDeleteEntityRequest(Worker_EntityId EntityId, TOptional<uint32> TimeoutMillis = {},
											 const TOptional<worker::c::Trace_SpanId>& SpanId = {});
	Worker_RequestId SendEntityQueryRequest(EntityQuery Query, TOptional<uint32> TimeoutMillis = {});
	Worker_RequestId SendEntityCommandRequest(Worker_EntityId EntityId, CommandRequest Request, TOptional<uint32> TimeoutMillis = {},
											  const TOptional<worker::c::Trace_SpanId>& SpanId = {});
	void SendEntityCommandResponse(Worker_RequestId RequestId, CommandResponse Response, const TOptional<worker::c::Trace_SpanId>& SpanId);
	void SendEntityCommandFailure(Worker_RequestId RequestId, FString Message, const TOptional<worker::c::Trace_SpanId>& SpanId);
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

	// Stored on ViewCoordinator to handle lifetime
	TSharedPtr<SpatialEventTracer> EventTracer;
};

} // namespace SpatialGDK
