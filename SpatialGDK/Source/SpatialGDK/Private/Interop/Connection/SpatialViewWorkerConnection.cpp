#include "Interop/Connection/SpatialViewWorkerConnection.h"

#include "SpatialGDKSettings.h"
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

} // anonymous namespace

void USpatialViewWorkerConnection::SetConnection(Worker_Connection* WorkerConnectionIn)
{
	TUniquePtr<SpatialGDK::SpatialOSConnectionHandler> Handler = MakeUnique<SpatialGDK::SpatialOSConnectionHandler>(WorkerConnectionIn);
	Coordinator = MakeUnique<SpatialGDK::ViewCoordinator>(MoveTemp(Handler));
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
	TArray<SpatialGDK::OpList> OpLists;
	OpLists.Add(Coordinator->Advance());
	return OpLists;
}

Worker_RequestId USpatialViewWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	check(Coordinator.IsValid());
	return Coordinator->SendReserveEntityIdsRequest(NumOfEntities);
}

Worker_RequestId USpatialViewWorkerConnection::SendCreateEntityRequest(TArray<FWorkerComponentData> Components,
																	   const Worker_EntityId* EntityId)
{
	check(Coordinator.IsValid());
	const TOptional<Worker_EntityId> Id = EntityId ? *EntityId : TOptional<Worker_EntityId>();
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
	return Coordinator->SendComponentUpdate(EntityId, ToComponentUpdate(ComponentUpdate));
}

Worker_RequestId USpatialViewWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
																  uint32_t CommandId)
{
	check(Coordinator.IsValid());
	return Coordinator->SendEntityCommandRequest(
		EntityId, SpatialGDK::CommandRequest(SpatialGDK::OwningCommandRequestPtr(Request->schema_type), Request->component_id,
											 Request->command_index));
}

void USpatialViewWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response)
{
	check(Coordinator.IsValid());
	Coordinator->SendEntityCommandResponse(
		RequestId, SpatialGDK::CommandResponse(SpatialGDK::OwningCommandResponsePtr(Response->schema_type), Response->component_id,
											   Response->command_index));
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

void USpatialViewWorkerConnection::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
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
		Coordinator->FlushMessagesToSend();
	}
}

void USpatialViewWorkerConnection::Flush()
{
	Coordinator->FlushMessagesToSend();
}
