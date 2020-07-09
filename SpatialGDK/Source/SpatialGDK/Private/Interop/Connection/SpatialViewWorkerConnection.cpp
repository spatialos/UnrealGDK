#include "Interop/Connection/SpatialViewWorkerConnection.h"


#include "Interop/Connection/OutgoingMessages.h"
#include "SpatialGDKSettings.h"
#include "SpatialView/ConnectionHandler/TracingConnectionHandler.h"
#include "SpatialView/CommandRequest.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/ConnectionHandler/SpatialOSConnectionHandler.h"
#include "SpatialView/ViewCoordinator.h"

namespace
{

SpatialGDK::ComponentData ToComponentData(FWorkerComponentData* Data)
{
	return SpatialGDK::ComponentData(SpatialGDK::OwningComponentDataPtr(Data->schema_type), Data->component_id);
}

SpatialGDK::ComponentUpdate ToComponentUpdate(FWorkerComponentUpdate* Update)
{
	return SpatialGDK::ComponentUpdate(SpatialGDK::OwningComponentUpdatePtr(Update->schema_type), Update->component_id);
}

}  // anonymous namespace

void USpatialViewWorkerConnection::SetConnection(Worker_Connection* WorkerConnectionIn)
{
	TUniquePtr<SpatialGDK::SpatialOSConnectionHandler> SpatialOSHandler = MakeUnique<SpatialGDK::SpatialOSConnectionHandler>(WorkerConnectionIn);
	TUniquePtr<SpatialGDK::TracingConnectionHandler> TracingHandler = MakeUnique<SpatialGDK::TracingConnectionHandler>(MoveTemp(SpatialOSHandler));
	TracingConnectionHandler = TracingHandler.Get();

	TracingHandler->AddCallback = [this](int32 TraceId)
	{
		FWorkerComponentData TraceOnly;
#if TRACE_LIB_ACTIVE
		TraceOnly.Trace = TraceId;
#endif  // TRACE_LIB_ACTIVE
		SpatialGDK::FAddComponent Data(0, TraceOnly);
		OnDequeueMessage.Broadcast(&Data);
	};

	TracingHandler->UpdateCallback = [this](int32 TraceId)
	{
		FWorkerComponentUpdate TraceOnly;
#if TRACE_LIB_ACTIVE
		TraceOnly.Trace = TraceId;
#endif  // TRACE_LIB_ACTIVE
		SpatialGDK::FComponentUpdate Update(0, TraceOnly);
		OnDequeueMessage.Broadcast(&Update);
	};

	TracingHandler->EntityCreationCallback = [this](TArray<int32> TraceIds)
	{
		TArray<FWorkerComponentData> TraceOnlyData;
		TraceOnlyData.Reserve(TraceIds.Num());
		for (int32 Id : TraceIds)
		{
			const int32 Index = TraceOnlyData.Emplace();
#if TRACE_LIB_ACTIVE
			TraceOnlyData[Index].Trace = Id;
#endif  // TRACE_LIB_ACTIVE
		}
		SpatialGDK::FCreateEntityRequest Request(MoveTemp(TraceOnlyData), nullptr);
		OnDequeueMessage.Broadcast(&Request);
	};
	Coordinator = MakeUnique<SpatialGDK::ViewCoordinator>(MoveTemp(TracingHandler));
}

void USpatialViewWorkerConnection::FinishDestroy()
{
	Coordinator.Reset();
	Super::FinishDestroy();
}

void USpatialViewWorkerConnection::DestroyConnection()
{
	Coordinator.Reset();
}

TArray<SpatialGDK::OpList> USpatialViewWorkerConnection::GetOpList()
{
	check(Coordinator.IsValid());
	Coordinator->FlushMessagesToSend();
	TArray<SpatialGDK::OpList> OpLists;
	OpLists.Add(Coordinator->Advance());
	return OpLists;
}

Worker_RequestId USpatialViewWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	check(Coordinator.IsValid());
	return Coordinator->SendReserveEntityIdsRequest(NumOfEntities);
}

Worker_RequestId USpatialViewWorkerConnection::SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const Worker_EntityId* EntityId)
{
	check(Coordinator.IsValid());
	if (EntityId != nullptr)
	{
		for (const FWorkerComponentData& Data : Components)
		{
			TracingConnectionHandler->SetEntityCreationTrace(*EntityId, Data.component_id, Data.Trace);
		}
	}

	SpatialGDK::FCreateEntityRequest TraceMessage(MoveTemp(Components), EntityId);
	OnEnqueueMessage.Broadcast(&TraceMessage);
	Components = MoveTemp(TraceMessage.Components);


	const TOptional<Worker_EntityId> Id = EntityId ? *EntityId  : TOptional<Worker_EntityId>();
	TArray<SpatialGDK::ComponentData> Data;
	Data.Reserve(Components.Num());
	for (auto& Component : Components)
	{
		Data.Emplace(SpatialGDK::OwningComponentDataPtr(Component.schema_type), Component.component_id);
	}
	return Coordinator->SendCreateEntityRequest(MoveTemp(Data), Id);
}

Worker_RequestId USpatialViewWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	check(Coordinator.IsValid());
	return Coordinator->SendDeleteEntityRequest(EntityId);
}

void USpatialViewWorkerConnection::SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData)
{
	check(Coordinator.IsValid());
	auto TraceMessage = SpatialGDK::FAddComponent(EntityId, *ComponentData);
	OnEnqueueMessage.Broadcast(&TraceMessage);
	if (ComponentData->Trace != 0)
	{
		TracingConnectionHandler->SetAddComponentTrace(EntityId, ComponentData->component_id, ComponentData->Trace);
	}

	return Coordinator->SendAddComponent(EntityId, ToComponentData(ComponentData));
}

void USpatialViewWorkerConnection::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	check(Coordinator.IsValid());
	return Coordinator->SendRemoveComponent(EntityId, ComponentId);
}

void USpatialViewWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate)
{
	check(Coordinator.IsValid());
	auto TraceMessage = SpatialGDK::FComponentUpdate(EntityId, *ComponentUpdate);
	OnEnqueueMessage.Broadcast(&TraceMessage);
	if (ComponentUpdate->Trace != 0)
	{
		TracingConnectionHandler->SetComponentUpdateTrace(EntityId, ComponentUpdate->component_id, ComponentUpdate->Trace);
	}

	return Coordinator->SendComponentUpdate(EntityId, ToComponentUpdate(ComponentUpdate));
}

Worker_RequestId USpatialViewWorkerConnection::SendCommandRequest(Worker_EntityId EntityId,
	Worker_CommandRequest* Request, uint32_t CommandId)
{
	check(Coordinator.IsValid());
	return Coordinator->SendEntityCommandRequest(EntityId, SpatialGDK::CommandRequest(
		SpatialGDK::OwningCommandRequestPtr(Request->schema_type) , Request->component_id, Request->command_index));
}

void USpatialViewWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response)
{
	check(Coordinator.IsValid());
	Coordinator->SendEntityCommandResponse(RequestId, SpatialGDK::CommandResponse(
		SpatialGDK::OwningCommandResponsePtr(Response->schema_type) , Response->component_id, Response->command_index));
}

void USpatialViewWorkerConnection::SendCommandFailure(Worker_RequestId RequestId, const FString& Message)
{
	check(Coordinator.IsValid());
	Coordinator->SendEntityCommandFailure(RequestId, Message);
}

void USpatialViewWorkerConnection::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{
	check(Coordinator.IsValid());
	Coordinator->SendLogMessage(static_cast<Worker_LogLevel>(Level), LoggerName, Message);
}

void USpatialViewWorkerConnection::SendComponentInterest(Worker_EntityId EntityId,
	TArray<Worker_InterestOverride>&& ComponentInterest)
{
	// Deprecated.
	checkNoEntry();
}

Worker_RequestId USpatialViewWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	check(Coordinator.IsValid());
	return Coordinator->SendEntityQueryRequest(SpatialGDK::EntityQuery(*EntityQuery));
}

void USpatialViewWorkerConnection::SendMetrics(SpatialGDK::SpatialMetrics Metrics)
{
	check(Coordinator.IsValid());
	Coordinator->SendMetrics(MoveTemp(Metrics));
}

PhysicalWorkerName USpatialViewWorkerConnection::GetWorkerId() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetWorkerId();
}

const TArray<FString>& USpatialViewWorkerConnection::GetWorkerAttributes() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetWorkerAttributes();
}

void USpatialViewWorkerConnection::ProcessOutgoingMessages()
{
	Coordinator->FlushMessagesToSend();
}

void USpatialViewWorkerConnection::MaybeFlush()
{
	const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>();
	if (Settings->bWorkerFlushAfterOutgoingNetworkOp)
	{
		Flush();
	}
}

void USpatialViewWorkerConnection::Flush()
{
	Coordinator->FlushMessagesToSend();
}
