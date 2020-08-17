#include "Interop/Connection/SpatialWorkerConnection.h"

#include "SpatialGDKSettings.h"
#include "SpatialView/CommandRequest.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/ConnectionHandler/InitialOpListConnectionHandler.h"
#include "SpatialView/ConnectionHandler/SpatialOSConnectionHandler.h"

DEFINE_LOG_CATEGORY(LogSpatialWorkerConnection);

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

void USpatialWorkerConnection::SetConnection(Worker_Connection* WorkerConnectionIn)
{
	StartupComplete = false;
	TUniquePtr<SpatialGDK::SpatialOSConnectionHandler> Handler = MakeUnique<SpatialGDK::SpatialOSConnectionHandler>(WorkerConnectionIn);
	TUniquePtr<SpatialGDK::InitialOpListConnectionHandler> InitialOpListHandler = MakeUnique<SpatialGDK::InitialOpListConnectionHandler>(
		MoveTemp(Handler), [this](SpatialGDK::OpList& Ops, SpatialGDK::ExtractedOpListData& ExtractedOps) {
			if (StartupComplete)
			{
				return true;
			}
			ExtractStartupOps(Ops, ExtractedOps);
			return false;
		});
	Coordinator = MakeUnique<SpatialGDK::ViewCoordinator>(MoveTemp(InitialOpListHandler));
}

void USpatialWorkerConnection::FinishDestroy()
{
	Coordinator.Reset();
	Super::FinishDestroy();
}

const TArray<SpatialGDK::EntityDelta>& USpatialWorkerConnection::GetEntityDeltas()
{
	check(Coordinator.IsValid());
	return Coordinator->GetViewDelta().GetEntityDeltas();
}

const TArray<Worker_Op>& USpatialWorkerConnection::GetWorkerMessages()
{
	check(Coordinator.IsValid());
	return Coordinator->GetViewDelta().GetWorkerMessages();
}

void USpatialWorkerConnection::DestroyConnection()
{
	Coordinator.Reset();
}

Worker_RequestId USpatialWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities)
{
	check(Coordinator.IsValid());
	return Coordinator->SendReserveEntityIdsRequest(NumOfEntities);
}

Worker_RequestId USpatialWorkerConnection::SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const Worker_EntityId* EntityId)
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

Worker_RequestId USpatialWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId)
{
	check(Coordinator.IsValid());
	return Coordinator->SendDeleteEntityRequest(EntityId);
}

void USpatialWorkerConnection::SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData)
{
	check(Coordinator.IsValid());
	Coordinator->SendAddComponent(EntityId, ToComponentData(ComponentData));
}

void USpatialWorkerConnection::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId)
{
	check(Coordinator.IsValid());
	Coordinator->SendRemoveComponent(EntityId, ComponentId);
}

void USpatialWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate)
{
	check(Coordinator.IsValid());
	Coordinator->SendComponentUpdate(EntityId, ToComponentUpdate(ComponentUpdate));
}

Worker_RequestId USpatialWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request, uint32_t CommandId)
{
	check(Coordinator.IsValid());
	return Coordinator->SendEntityCommandRequest(
		EntityId, SpatialGDK::CommandRequest(SpatialGDK::OwningCommandRequestPtr(Request->schema_type), Request->component_id,
											 Request->command_index));
}

void USpatialWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response)
{
	check(Coordinator.IsValid());
	Coordinator->SendEntityCommandResponse(
		RequestId, SpatialGDK::CommandResponse(SpatialGDK::OwningCommandResponsePtr(Response->schema_type), Response->component_id,
											   Response->command_index));
}

void USpatialWorkerConnection::SendCommandFailure(Worker_RequestId RequestId, const FString& Message)
{
	check(Coordinator.IsValid());
	Coordinator->SendEntityCommandFailure(RequestId, Message);
}

void USpatialWorkerConnection::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{
	check(Coordinator.IsValid());
	Coordinator->SendLogMessage(static_cast<Worker_LogLevel>(Level), LoggerName, Message);
}

void USpatialWorkerConnection::SendComponentInterest(Worker_EntityId EntityId, TArray<Worker_InterestOverride>&& ComponentInterest)
{
	// Deprecated.
	checkNoEntry();
}

Worker_RequestId USpatialWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery)
{
	check(Coordinator.IsValid());
	return Coordinator->SendEntityQueryRequest(SpatialGDK::EntityQuery(*EntityQuery));
}

void USpatialWorkerConnection::SendMetrics(SpatialGDK::SpatialMetrics Metrics)
{
	check(Coordinator.IsValid());
	Coordinator->SendMetrics(MoveTemp(Metrics));
}

void USpatialWorkerConnection::Advance()
{
	check(Coordinator.IsValid());
	Coordinator->Advance();
}

bool USpatialWorkerConnection::HasDisconnected() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetViewDelta().HasDisconnected();
}

Worker_ConnectionStatusCode USpatialWorkerConnection::GetConnectionStatus() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetViewDelta().GetConnectionStatus();
}

FString USpatialWorkerConnection::GetDisconnectReason() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetViewDelta().GetDisconnectReason();
}

const SpatialGDK::EntityView& USpatialWorkerConnection::GetView() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetView();
}

PhysicalWorkerName USpatialWorkerConnection::GetWorkerId() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetWorkerId();
}

const TArray<FString>& USpatialWorkerConnection::GetWorkerAttributes() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetWorkerAttributes();
}

void USpatialWorkerConnection::Flush()
{
	Coordinator->FlushMessagesToSend();
}

void USpatialWorkerConnection::SetStartupComplete()
{
	StartupComplete = true;
}

bool USpatialWorkerConnection::IsStartupComponent(Worker_ComponentId Id)
{
	return Id == SpatialConstants::STARTUP_ACTOR_MANAGER_COMPONENT_ID || Id == SpatialConstants::VIRTUAL_WORKER_TRANSLATION_COMPONENT_ID
		   || Id == SpatialConstants::SERVER_WORKER_COMPONENT_ID;
}

void USpatialWorkerConnection::ExtractStartupOps(SpatialGDK::OpList& OpList, SpatialGDK::ExtractedOpListData& ExtractedOpList)
{
	for (uint32 i = 0; i < OpList.Count; ++i)
	{
		Worker_Op& Op = OpList.Ops[i];
		switch (static_cast<Worker_OpType>(Op.op_type))
		{
		case WORKER_OP_TYPE_ADD_ENTITY:
			ExtractedOpList.AddOp(Op);
			break;
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			ExtractedOpList.AddOp(Op);
			break;
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			ExtractedOpList.AddOp(Op);
			break;
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			ExtractedOpList.AddOp(Op);
			break;
		case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
			ExtractedOpList.AddOp(Op);
			break;
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			ExtractedOpList.AddOp(Op);
			break;
		case WORKER_OP_TYPE_ADD_COMPONENT:
			if (IsStartupComponent(Op.op.add_component.data.component_id))
			{
				ExtractedOpList.AddOp(Op);
			}
			break;
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			if (IsStartupComponent(Op.op.remove_component.component_id))
			{
				ExtractedOpList.AddOp(Op);
			}
			break;
		case WORKER_OP_TYPE_AUTHORITY_CHANGE:
			if (IsStartupComponent(Op.op.authority_change.component_id))
			{
				ExtractedOpList.AddOp(Op);
			}
			break;
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			if (IsStartupComponent(Op.op.component_update.update.component_id))
			{
				ExtractedOpList.AddOp(Op);
			}
			break;
		case WORKER_OP_TYPE_COMMAND_REQUEST:
			break;
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
			ExtractedOpList.AddOp(Op);
			break;
		case WORKER_OP_TYPE_DISCONNECT:
			ExtractedOpList.AddOp(Op);
			break;
		case WORKER_OP_TYPE_FLAG_UPDATE:
			break;
		case WORKER_OP_TYPE_LOG_MESSAGE:
			break;
		case WORKER_OP_TYPE_METRICS:
			break;
		case WORKER_OP_TYPE_CRITICAL_SECTION:
			break;
		default:
			break;
		}
	}
}
