// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOSWorkerConnectionSpy.h"

#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/ViewDelta.h"
#include "Utils/SpatialLatencyTracer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

SpatialOSWorkerConnectionSpy::SpatialOSWorkerConnectionSpy()
	: NextRequestId(0)
	, LastEntityQuery(nullptr)
{
}

const TArray<SpatialGDK::EntityDelta>& SpatialOSWorkerConnectionSpy::GetEntityDeltas()
{
	return PlaceholderEntityDeltas;
}

const TArray<Worker_Op>& SpatialOSWorkerConnectionSpy::GetWorkerMessages()
{
	return PlaceholderWorkerMessages;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendCreateEntityRequest(TArray<FWorkerComponentData> Components,
																	   const Worker_EntityId* EntityId,
																	   const TOptional<Trace_SpanId>& SpanId)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendDeleteEntityRequest(Worker_EntityId EntityId, const TOptional<Trace_SpanId>& SpanId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData,
													const TOptional<Trace_SpanId>& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
													   const TOptional<Trace_SpanId>& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate,
													   const TOptional<Trace_SpanId>& SpanId)
{
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
																  uint32_t CommandId, const TOptional<Trace_SpanId>& SpanId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response,
													   const TOptional<Trace_SpanId>& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendCommandFailure(Worker_RequestId RequestId, const FString& Message,
													  const TOptional<Trace_SpanId>& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) {}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	LastEntityQuery = EntityQuery;
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendMetrics(SpatialGDK::SpatialMetrics Metrics) {}

const Worker_EntityQuery* SpatialOSWorkerConnectionSpy::GetLastEntityQuery()
{
	return LastEntityQuery;
}

void SpatialOSWorkerConnectionSpy::ClearLastEntityQuery()
{
	LastEntityQuery = nullptr;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::GetLastRequestId()
{
	return NextRequestId - 1;
}
