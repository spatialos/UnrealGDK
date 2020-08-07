// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOSWorkerConnectionSpy.h"

#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialCommonTypes.h"
#include "Utils/SpatialLatencyTracer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

SpatialOSWorkerConnectionSpy::SpatialOSWorkerConnectionSpy()
	: NextRequestId(0)
	, LastEntityQuery(nullptr)
{
}

TArray<SpatialGDK::OpList> SpatialOSWorkerConnectionSpy::GetOpList()
{
	return TArray<SpatialGDK::OpList>();
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendCreateEntityRequest(TArray<FWorkerComponentData> Components,
																	   const Worker_EntityId* EntityId)
{
	return NextRequestId++;
}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData) {}

void SpatialOSWorkerConnectionSpy::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId) {}

void SpatialOSWorkerConnectionSpy::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate) {}

Worker_RequestId SpatialOSWorkerConnectionSpy::SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
																  uint32_t CommandId)
{
	return NextRequestId++;
}

void SpatialOSWorkerConnectionSpy::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response) {}

void SpatialOSWorkerConnectionSpy::SendCommandFailure(Worker_RequestId RequestId, const FString& Message) {}

void SpatialOSWorkerConnectionSpy::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message) {}

void SpatialOSWorkerConnectionSpy::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest) {}

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
