// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Set.h"
#include "SpatialView/MessagesToSend.h"
#include "SpatialView/OpList/OpList.h"
#include "SpatialView/ViewDelta.h"

namespace SpatialGDK
{

class SpatialEventTracer;

class WorkerView
{
public:
	explicit WorkerView(SpatialEventTracer* InEventTracer);

	// Process queued op lists to create a new view delta.
	// The view delta will exist until the next call to advance.
	ViewDelta GenerateViewDelta();

	// Add an OpList to generate the next ViewDelta.
	void EnqueueOpList(OpList Ops);

	// Ensure all local changes have been applied and return the resulting MessagesToSend.
	TUniquePtr<MessagesToSend> FlushLocalChanges();

	void SendAddComponent(Worker_EntityId EntityId, ComponentData Data, const TOptional<worker::c::Trace_SpanId>& SpanId);
	void SendComponentUpdate(Worker_EntityId EntityId, ComponentUpdate Update, const TOptional<worker::c::Trace_SpanId>& SpanId);
	void SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId, const TOptional<worker::c::Trace_SpanId>& SpanId);
	void SendReserveEntityIdsRequest(ReserveEntityIdsRequest Request);
	void SendCreateEntityRequest(CreateEntityRequest Request);
	void SendDeleteEntityRequest(DeleteEntityRequest Request);
	void SendEntityQueryRequest(EntityQueryRequest Request);
	void SendEntityCommandRequest(EntityCommandRequest Request);
	void SendEntityCommandResponse(EntityCommandResponse Response);
	void SendEntityCommandFailure(EntityCommandFailure Failure);
	void SendMetrics(SpatialMetrics Metrics);
	void SendLogMessage(LogMessage Log);

private:
	EntityView View;

	TArray<OpList> QueuedOps;
	TArray<OpList> OpenCriticalSectionOps;

	TUniquePtr<MessagesToSend> LocalChanges;

	SpatialEventTracer* EventTracer;
};

} // namespace SpatialGDK
