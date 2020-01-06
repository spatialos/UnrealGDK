// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "WorkerConnection.h"
#include "SpatialWorkerConnection.h"

void UWorkerConnection::DestroyConnection()
{
}

PhysicalWorkerName UWorkerConnection::GetWorkerId() const
{
	return PhysicalWorkerName{};
}

Worker_RequestId UWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId)
{
	return 0;
}

void UWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate, const TraceKey Key /*= USpatialLatencyTracer::InvalidTraceKey*/)
{
}

Worker_RequestId UWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	return 0;
}

Worker_RequestId UWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	return 0;
}

Worker_RequestId UWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	return 0;
}

Worker_RequestId UWorkerConnection::SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId)
{
	return 0;
}

void UWorkerConnection::SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData)
{
}

void UWorkerConnection::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
}

void UWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{
}

void UWorkerConnection::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
{
}

void UWorkerConnection::SendMetrics(const SpatialGDK::SpatialMetrics& Metrics)
{
}

void UWorkerConnection::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{
}

const TArray<FString>& UWorkerConnection::GetWorkerAttributes() const
{
	return WorkerAttributes;
}

void UWorkerConnection::SetConnectionType(ESpatialConnectionType InConnectionType)
{
}

void UWorkerConnection::Connect(bool bConnectAsClient, uint32 PlayInEditorID)
{
}

bool UWorkerConnection::IsConnected() const
{
	return false;
}

TArray<Worker_OpList*> UWorkerConnection::GetOpList()
{
	return TArray<Worker_OpList*>{};
}

bool UWorkerConnection::Init()
{
	return true;
}

uint32 UWorkerConnection::Run()
{
	return 0;
}

void UWorkerConnection::Stop()
{
}
