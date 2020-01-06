// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "WorkerConnection.h"
#include "SpatialWorkerConnection.h"

UWorkerConnection::UWorkerConnection(const FObjectInitializer & ObjectInitializer /*= FObjectInitializer::Get()*/)
{
	WorkerConnectionImpl = NewObject<USpatialWorkerConnection>();
}

void UWorkerConnection::DestroyConnection()
{
	// TODO(Alex): destroy WorkerConnectionImpl?
}

PhysicalWorkerName UWorkerConnection::GetWorkerId() const
{
	return WorkerConnectionImpl->GetWorkerId();
}

Worker_RequestId UWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, const Worker_CommandRequest* Request, uint32_t CommandId)
{
	return WorkerConnectionImpl->SendCommandRequest(EntityId, Request, CommandId);
}

void UWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, const Worker_ComponentUpdate* ComponentUpdate, const TraceKey Key /*= USpatialLatencyTracer::InvalidTraceKey*/)
{
	WorkerConnectionImpl->SendComponentUpdate(EntityId, ComponentUpdate, Key);
}

Worker_RequestId UWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	return WorkerConnectionImpl->SendEntityQueryRequest(EntityQuery);
}

Worker_RequestId UWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	return WorkerConnectionImpl->SendReserveEntityIdsRequest(NumOfEntities);
}

Worker_RequestId UWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	return WorkerConnectionImpl->SendDeleteEntityRequest(EntityId);
}

Worker_RequestId UWorkerConnection::SendCreateEntityRequest(TArray<Worker_ComponentData>&& Components, const Worker_EntityId* EntityId)
{
	return WorkerConnectionImpl->SendCreateEntityRequest(MoveTemp(Components), EntityId);
}

void UWorkerConnection::SendAddComponent(Worker_EntityId EntityId, Worker_ComponentData* ComponentData)
{
	WorkerConnectionImpl->SendAddComponent(EntityId, ComponentData);
}

void UWorkerConnection::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	WorkerConnectionImpl->SendRemoveComponent(EntityId, ComponentId);
}

void UWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, const Worker_CommandResponse* Response)
{
	WorkerConnectionImpl->SendCommandResponse(RequestId, Response);
}

void UWorkerConnection::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
{
	WorkerConnectionImpl->SendComponentInterest(EntityId, MoveTemp(ComponentInterest));
}

void UWorkerConnection::SendMetrics(const SpatialGDK::SpatialMetrics& Metrics)
{
	WorkerConnectionImpl->SendMetrics(Metrics);
}

void UWorkerConnection::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{
	WorkerConnectionImpl->SendLogMessage(Level, LoggerName, Message);
}

const TArray<FString>& UWorkerConnection::GetWorkerAttributes() const
{
	return WorkerConnectionImpl->GetWorkerAttributes();
}

void UWorkerConnection::SetConnectionType(ESpatialConnectionType InConnectionType)
{
	WorkerConnectionImpl->SetConnectionType(InConnectionType);
}

void UWorkerConnection::Connect(bool bConnectAsClient, uint32 PlayInEditorID)
{
	WorkerConnectionImpl->Connect(bConnectAsClient, PlayInEditorID);
}

bool UWorkerConnection::IsConnected() const
{
	return WorkerConnectionImpl->IsConnected();
}

TArray<Worker_OpList*> UWorkerConnection::GetOpList()
{
	return WorkerConnectionImpl->GetOpList();
}

bool UWorkerConnection::Init()
{
	return WorkerConnectionImpl->Init();
}

uint32 UWorkerConnection::Run()
{
	return WorkerConnectionImpl->Run();
}

void UWorkerConnection::Stop()
{
	WorkerConnectionImpl->Stop();
}
