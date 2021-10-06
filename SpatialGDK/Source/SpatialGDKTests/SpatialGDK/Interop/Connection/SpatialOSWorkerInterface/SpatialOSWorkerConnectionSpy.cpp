// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOSWorkerConnectionSpy.h"

#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/ViewDelta.h"
#include "Utils/SpatialLatencyTracer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialView/CommandRetryHandler.h"

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

Worker_RequestId SpatialOSWorkerConnectionSpy::SendReserveEntityIdsRequest(uint32_t NumOfEntities, const SpatialGDK::FRetryData& RetryData)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendCreateEntityRequest(TArray<FWorkerComponentData> Components,
																	   const Worker_EntityId* EntityId,
																	   const SpatialGDK::FRetryData& RetryData,
																	   const FSpatialGDKSpanId& SpanId)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendDeleteEntityRequest(Worker_EntityId EntityId, const SpatialGDK::FRetryData& RetryData,
																	   const FSpatialGDKSpanId& SpanId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData,
													const FSpatialGDKSpanId& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
													   const FSpatialGDKSpanId& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate,
													   const FSpatialGDKSpanId& SpanId)
{
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
																  const SpatialGDK::FRetryData& RetryData, const FSpatialGDKSpanId& SpanId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response,
													   const FSpatialGDKSpanId& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const FSpatialGDKSpanId& SpanId)
{
}

void SpatialOSWorkerConnectionSpy::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) {}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery,
																	  const SpatialGDK::FRetryData& RetryData)
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
