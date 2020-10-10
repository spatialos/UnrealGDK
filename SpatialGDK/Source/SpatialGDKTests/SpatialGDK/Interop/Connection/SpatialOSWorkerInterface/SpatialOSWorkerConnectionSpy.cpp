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

FRequestId SpatialOSWorkerConnectionSpy::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	return NextRequestId++;
}

FRequestId SpatialOSWorkerConnectionSpy::SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const FEntityId* EntityId,
																 const TOptional<Trace_SpanId>& SpanId)
{
	return NextRequestId++;
}

FRequestId SpatialOSWorkerConnectionSpy::SendDeleteEntityRequest(FEntityId EntityId, const TOptional<Trace_SpanId>& SpanId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendAddComponent(FEntityId EntityId, FWorkerComponentData* ComponentData,
													const TOptional<Trace_SpanId>& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendRemoveComponent(FEntityId EntityId, Worker_ComponentId ComponentId,
													   const TOptional<Trace_SpanId>& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendComponentUpdate(FEntityId EntityId, FWorkerComponentUpdate* ComponentUpdate,
													   const TOptional<Trace_SpanId>& SpanId)
{
}

FRequestId SpatialOSWorkerConnectionSpy::SendCommandRequest(FEntityId EntityId, Worker_CommandRequest* Request, uint32_t CommandId,
															const TOptional<Trace_SpanId>& SpanId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendCommandResponse(FRequestId RequestId, Worker_CommandResponse* Response,
													   const TOptional<Trace_SpanId>& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendCommandFailure(FRequestId RequestId, const FString& Message, const TOptional<Trace_SpanId>& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) {}

void SpatialOSWorkerConnectionSpy::SendComponentInterest(FEntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest) {}

FRequestId SpatialOSWorkerConnectionSpy::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
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

FRequestId SpatialOSWorkerConnectionSpy::GetLastRequestId()
{
	return NextRequestId - 1;
}
