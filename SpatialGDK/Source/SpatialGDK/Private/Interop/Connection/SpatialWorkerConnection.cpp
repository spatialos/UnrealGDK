#include "Interop/Connection/SpatialWorkerConnection.h"

#include "Interop/Connection/SpatialEventTracer.h"
#include "SpatialGDKSettings.h"
#include "SpatialView/CommandRequest.h"
#include "SpatialView/CommandRetryHandler.h"
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

void USpatialWorkerConnection::SetConnection(Worker_Connection* WorkerConnectionIn,
											 TSharedPtr<SpatialGDK::SpatialEventTracer> SharedEventTracer,
											 SpatialGDK::FComponentSetData ComponentSetData)
{
	EventTracer = SharedEventTracer.Get();
	StartupComplete = false;
	TUniquePtr<SpatialGDK::SpatialOSConnectionHandler> Handler =
		MakeUnique<SpatialGDK::SpatialOSConnectionHandler>(WorkerConnectionIn, SharedEventTracer);
	TUniquePtr<SpatialGDK::InitialOpListConnectionHandler> InitialOpListHandler = MakeUnique<SpatialGDK::InitialOpListConnectionHandler>(
		MoveTemp(Handler), [this](SpatialGDK::OpList& Ops, SpatialGDK::ExtractedOpListData& ExtractedOps) {
			if (StartupComplete)
			{
				return true;
			}
			ExtractStartupOps(Ops, ExtractedOps);
			return false;
		});
	Coordinator = MakeUnique<SpatialGDK::ViewCoordinator>(MoveTemp(InitialOpListHandler), SharedEventTracer, MoveTemp(ComponentSetData));
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

Worker_RequestId USpatialWorkerConnection::SendReserveEntityIdsRequest(uint32_t NumOfEntities, const SpatialGDK::FRetryData& RetryData)
{
	check(Coordinator.IsValid());
	return Coordinator->SendReserveEntityIdsRequest(NumOfEntities, RetryData);
}

Worker_RequestId USpatialWorkerConnection::SendCreateEntityRequest(TArray<FWorkerComponentData> Components, const Worker_EntityId* EntityId,
																   const SpatialGDK::FRetryData& RetryData, const FSpatialGDKSpanId& SpanId)
{
	check(Coordinator.IsValid());
	const TOptional<Worker_EntityId> Id = EntityId != nullptr ? *EntityId : TOptional<Worker_EntityId>();
	TArray<SpatialGDK::ComponentData> Data;
	Data.Reserve(Components.Num());
	for (auto& Component : Components)
	{
		Data.Emplace(SpatialGDK::OwningComponentDataPtr(Component.schema_type), Component.component_id);
	}

	return Coordinator->SendCreateEntityRequest(MoveTemp(Data), Id, RetryData, SpanId);
}

Worker_RequestId USpatialWorkerConnection::SendDeleteEntityRequest(Worker_EntityId EntityId, const SpatialGDK::FRetryData& RetryData,
																   const FSpatialGDKSpanId& SpanId)
{
	check(Coordinator.IsValid());
	return Coordinator->SendDeleteEntityRequest(EntityId, RetryData, SpanId);
}

void USpatialWorkerConnection::SendAddComponent(Worker_EntityId EntityId, FWorkerComponentData* ComponentData,
												const FSpatialGDKSpanId& SpanId)
{
	check(Coordinator.IsValid());
	Coordinator->SendAddComponent(EntityId, ToComponentData(ComponentData), SpanId);
}

void USpatialWorkerConnection::SendRemoveComponent(Worker_EntityId EntityId, Worker_ComponentId ComponentId,
												   const FSpatialGDKSpanId& SpanId)
{
	check(Coordinator.IsValid());
	Coordinator->SendRemoveComponent(EntityId, ComponentId, SpanId);
}

void USpatialWorkerConnection::SendComponentUpdate(Worker_EntityId EntityId, FWorkerComponentUpdate* ComponentUpdate,
												   const FSpatialGDKSpanId& SpanId)
{
	check(Coordinator.IsValid());
	Coordinator->SendComponentUpdate(EntityId, ToComponentUpdate(ComponentUpdate), SpanId);
}

Worker_RequestId USpatialWorkerConnection::SendCommandRequest(Worker_EntityId EntityId, Worker_CommandRequest* Request,
															  const SpatialGDK::FRetryData& RetryData, const FSpatialGDKSpanId& SpanId)
{
	check(Coordinator.IsValid());
	return Coordinator->SendEntityCommandRequest(EntityId,
												 SpatialGDK::CommandRequest(SpatialGDK::OwningCommandRequestPtr(Request->schema_type),
																			Request->component_id, Request->command_index),
												 RetryData, SpanId);
}

void USpatialWorkerConnection::SendCommandResponse(Worker_RequestId RequestId, Worker_CommandResponse* Response,
												   const FSpatialGDKSpanId& SpanId)
{
	check(Coordinator.IsValid());
	Coordinator->SendEntityCommandResponse(RequestId,
										   SpatialGDK::CommandResponse(SpatialGDK::OwningCommandResponsePtr(Response->schema_type),
																	   Response->component_id, Response->command_index),
										   SpanId);
}

void USpatialWorkerConnection::SendCommandFailure(Worker_RequestId RequestId, const FString& Message, const FSpatialGDKSpanId& SpanId)
{
	check(Coordinator.IsValid());
	Coordinator->SendEntityCommandFailure(RequestId, Message, SpanId);
}

void USpatialWorkerConnection::SendLogMessage(uint8_t Level, const FName& LoggerName, const TCHAR* Message)
{
	check(Coordinator.IsValid());
	Coordinator->SendLogMessage(static_cast<Worker_LogLevel>(Level), LoggerName, Message);
}

Worker_RequestId USpatialWorkerConnection::SendEntityQueryRequest(const Worker_EntityQuery* EntityQuery,
																  const SpatialGDK::FRetryData& RetryData)
{
	check(Coordinator.IsValid());
	return Coordinator->SendEntityQueryRequest(SpatialGDK::EntityQuery(*EntityQuery), RetryData);
}

void USpatialWorkerConnection::SendMetrics(SpatialGDK::SpatialMetrics Metrics)
{
	check(Coordinator.IsValid());
	Coordinator->SendMetrics(MoveTemp(Metrics));
}

void USpatialWorkerConnection::Advance(float DeltaTimeS)
{
	check(Coordinator.IsValid());
	Coordinator->Advance(DeltaTimeS);
}

bool USpatialWorkerConnection::HasDisconnected() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetViewDelta().HasConnectionStatusChanged();
}

Worker_ConnectionStatusCode USpatialWorkerConnection::GetConnectionStatus() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetViewDelta().GetConnectionStatusChange();
}

FString USpatialWorkerConnection::GetDisconnectReason() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetViewDelta().GetConnectionStatusChangeMessage();
}

const SpatialGDK::EntityView& USpatialWorkerConnection::GetView() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetView();
}

SpatialGDK::ViewCoordinator& USpatialWorkerConnection::GetCoordinator() const
{
	return *Coordinator;
}

PhysicalWorkerName USpatialWorkerConnection::GetWorkerId() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetWorkerId();
}

Worker_EntityId USpatialWorkerConnection::GetWorkerSystemEntityId() const
{
	check(Coordinator.IsValid());
	return Coordinator->GetWorkerSystemEntityId();
}

SpatialGDK::CallbackId USpatialWorkerConnection::RegisterComponentAddedCallback(Worker_ComponentId ComponentId,
																				SpatialGDK::FComponentValueCallback Callback)
{
	check(Coordinator.IsValid());
	return Coordinator->RegisterComponentAddedCallback(ComponentId, MoveTemp(Callback));
}

SpatialGDK::CallbackId USpatialWorkerConnection::RegisterComponentRemovedCallback(Worker_ComponentId ComponentId,
																				  SpatialGDK::FComponentValueCallback Callback)
{
	check(Coordinator.IsValid());
	return Coordinator->RegisterComponentRemovedCallback(ComponentId, MoveTemp(Callback));
}

SpatialGDK::CallbackId USpatialWorkerConnection::RegisterComponentValueCallback(Worker_ComponentId ComponentId,
																				SpatialGDK::FComponentValueCallback Callback)
{
	check(Coordinator.IsValid());
	return Coordinator->RegisterComponentValueCallback(ComponentId, MoveTemp(Callback));
}

SpatialGDK::CallbackId USpatialWorkerConnection::RegisterAuthorityGainedCallback(Worker_ComponentId ComponentId,
																				 SpatialGDK::FEntityCallback Callback)
{
	check(Coordinator.IsValid());
	return Coordinator->RegisterAuthorityGainedCallback(ComponentId, MoveTemp(Callback));
}

SpatialGDK::CallbackId USpatialWorkerConnection::RegisterAuthorityLostCallback(Worker_ComponentId ComponentId,
																			   SpatialGDK::FEntityCallback Callback)
{
	check(Coordinator.IsValid());
	return Coordinator->RegisterAuthorityLostCallback(ComponentId, MoveTemp(Callback));
}

SpatialGDK::CallbackId USpatialWorkerConnection::RegisterAuthorityLostTempCallback(Worker_ComponentId ComponentId,
																				   SpatialGDK::FEntityCallback Callback)
{
	check(Coordinator.IsValid());
	return Coordinator->RegisterAuthorityLostTempCallback(ComponentId, MoveTemp(Callback));
}

void USpatialWorkerConnection::RemoveCallback(SpatialGDK::CallbackId Id)
{
	check(Coordinator.IsValid());
	Coordinator->RemoveCallback(Id);
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
		   || Id == SpatialConstants::SERVER_WORKER_COMPONENT_ID || Id == SpatialConstants::GDK_KNOWN_ENTITY_TAG_COMPONENT_ID;
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
		case WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE:
			if (IsStartupComponent(Op.op.component_set_authority_change.component_set_id))
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
		case WORKER_OP_TYPE_METRICS:
			break;
		case WORKER_OP_TYPE_CRITICAL_SECTION:
			break;
		default:
			break;
		}
	}
}
