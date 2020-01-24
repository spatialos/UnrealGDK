// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOSWorkerConnectionMock.h"

#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialCommonTypes.h"
#include "Utils/SpatialLatencyTracer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

SpatialOSWorkerConnectionMock::SpatialOSWorkerConnectionMock()
	: NextRequestId(0)
	, LastEntityQuery(nullptr)
{}

Worker_RequestId SpatialOSWorkerConnectionMock::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionMock::SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionMock::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionMock::SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData, const TraceKey Key)
{}

void SpatialOSWorkerConnectionMock::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{}

void SpatialOSWorkerConnectionMock::SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate, const TraceKey Key)
{}

Worker_RequestId SpatialOSWorkerConnectionMock::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionMock::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{}

void SpatialOSWorkerConnectionMock::SendCommandFailure(Worker_RequestId RequestId, const FString& Message)
{}

void SpatialOSWorkerConnectionMock::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{}

void SpatialOSWorkerConnectionMock::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
{}

Worker_RequestId SpatialOSWorkerConnectionMock::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	LastEntityQuery = EntityQuery;
	return NextRequestId++;
}

void SpatialOSWorkerConnectionMock::SendMetrics(const SpatialGDK::SpatialMetrics& Metrics)
{}

const Worker_EntityQuery* SpatialOSWorkerConnectionMock::GetLastEntityQuery()
{
	return LastEntityQuery;
}

void SpatialOSWorkerConnectionMock::ClearLastEntityQuery()
{
	LastEntityQuery = nullptr;
}

Worker_RequestId SpatialOSWorkerConnectionMock::GetLastRequestId()
{
	return NextRequestId - 1;
}
