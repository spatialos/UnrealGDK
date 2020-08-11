// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ConnectionHandler/SpatialOSConnectionHandler.h"

namespace SpatialGDK
{
SpatialOSConnectionHandler::SpatialOSConnectionHandler(Worker_Connection* Connection)
	: Connection(Connection)
	, WorkerId(UTF8_TO_TCHAR(Worker_Connection_GetWorkerId(Connection)))
{
	const Worker_WorkerAttributes* Attributes = Worker_Connection_GetWorkerAttributes(Connection);
	for (uint32 i = 0; i < Attributes->attribute_count; ++i)
	{
		WorkerAttributes.Push(FString(UTF8_TO_TCHAR(Attributes->attributes[i])));
	}
}

void SpatialOSConnectionHandler::Advance() {}

uint32 SpatialOSConnectionHandler::GetOpListCount()
{
	return 1;
}

OpList SpatialOSConnectionHandler::GetNextOpList()
{
	OpList Ops = GetOpListFromConnection(Connection.Get());
	// Find responses and change their internal request IDs to match the request ID provided by the user.
	for (uint32 i = 0; i < Ops.Count; ++i)
	{
		Worker_Op& Op = Ops.Ops[i];
		Worker_RequestId* Id;
		switch (static_cast<Worker_OpType>(Op.op_type))
		{
		case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
			Id = &Op.op.reserve_entity_ids_response.request_id;
			break;
		case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
			Id = &Op.op.create_entity_response.request_id;
			break;
		case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
			Id = &Op.op.delete_entity_response.request_id;
			break;
		case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
			Id = &Op.op.entity_query_response.request_id;
			break;
		case WORKER_OP_TYPE_COMMAND_RESPONSE:
			Id = &Op.op.command_response.request_id;
			break;
		default:
			Id = nullptr;
			break;
		}

		if (Id != nullptr)
		{
			*Id = InternalToUserRequestId.FindAndRemoveChecked(*Id);
		}
	}

	return Ops;
}

void SpatialOSConnectionHandler::SendMessages(TUniquePtr<MessagesToSend> Messages)
{
	const Worker_UpdateParameters UpdateParams = { 0 /*loopback*/ };
	const Worker_CommandParameters CommandParams = { 0 /*allow_short_circuit*/ };
	for (auto& Message : Messages->ComponentMessages)
	{
		switch (Message.GetType())
		{
		case OutgoingComponentMessage::ADD:
		{
			Worker_ComponentData Data = { nullptr, Message.ComponentId, MoveTemp(Message).ReleaseComponentAdded().Release(), nullptr };
			Worker_Connection_SendAddComponent(Connection.Get(), Message.EntityId, &Data, &UpdateParams);
			break;
		}
		case OutgoingComponentMessage::UPDATE:
		{
			Worker_ComponentUpdate Update = { nullptr, Message.ComponentId, MoveTemp(Message).ReleaseComponentUpdate().Release(), nullptr };
			Worker_Connection_SendComponentUpdate(Connection.Get(), Message.EntityId, &Update, &UpdateParams);
			break;
		}
		case OutgoingComponentMessage::REMOVE:
		{
			Worker_Connection_SendRemoveComponent(Connection.Get(), Message.EntityId, Message.ComponentId, &UpdateParams);
			break;
		}
		default:
			checkNoEntry();
			break;
		}
	}

	for (auto& Request : Messages->ReserveEntityIdsRequests)
	{
		const uint32* Timeout = Request.TimeoutMillis.IsSet() ? &Request.TimeoutMillis.GetValue() : nullptr;
		const Worker_RequestId Id = Worker_Connection_SendReserveEntityIdsRequest(Connection.Get(), Request.NumberOfEntityIds, Timeout);
		InternalToUserRequestId.Emplace(Id, Request.RequestId);
	}

	for (auto& Request : Messages->CreateEntityRequests)
	{
		TArray<Worker_ComponentData> Components;
		Components.Reserve(Request.EntityComponents.Num());
		for (ComponentData& Component : Request.EntityComponents)
		{
			Components.Push(Worker_ComponentData{ nullptr, Component.GetComponentId(), MoveTemp(Component).Release(), nullptr });
		}
		Worker_EntityId* EntityId = Request.EntityId.IsSet() ? &Request.EntityId.GetValue() : nullptr;
		const uint32* Timeout = Request.TimeoutMillis.IsSet() ? &Request.TimeoutMillis.GetValue() : nullptr;
		const Worker_RequestId Id =
			Worker_Connection_SendCreateEntityRequest(Connection.Get(), Components.Num(), Components.GetData(), EntityId, Timeout);
		InternalToUserRequestId.Emplace(Id, Request.RequestId);
	}

	for (auto& Request : Messages->DeleteEntityRequests)
	{
		const uint32* Timeout = Request.TimeoutMillis.IsSet() ? &Request.TimeoutMillis.GetValue() : nullptr;
		const Worker_RequestId Id = Worker_Connection_SendDeleteEntityRequest(Connection.Get(), Request.EntityId, Timeout);
		InternalToUserRequestId.Emplace(Id, Request.RequestId);
	}

	for (auto& Request : Messages->EntityQueryRequests)
	{
		const uint32* Timeout = Request.TimeoutMillis.IsSet() ? &Request.TimeoutMillis.GetValue() : nullptr;
		Worker_EntityQuery Query = Request.Query.GetWorkerQuery();
		const Worker_RequestId Id = Worker_Connection_SendEntityQueryRequest(Connection.Get(), &Query, Timeout);
		InternalToUserRequestId.Emplace(Id, Request.RequestId);
	}

	for (auto& Request : Messages->EntityCommandRequests)
	{
		const uint32* Timeout = Request.TimeoutMillis.IsSet() ? &Request.TimeoutMillis.GetValue() : nullptr;
		Worker_CommandRequest r = { nullptr, Request.Request.GetComponentId(), Request.Request.GetCommandIndex(),
									MoveTemp(Request.Request).Release(), nullptr };
		const Worker_RequestId Id = Worker_Connection_SendCommandRequest(Connection.Get(), Request.EntityId, &r, Timeout, &CommandParams);
		InternalToUserRequestId.Emplace(Id, Request.RequestId);
	}

	for (auto& Response : Messages->EntityCommandResponses)
	{
		Worker_CommandResponse r = { nullptr, Response.Response.GetComponentId(), Response.Response.GetCommandIndex(),
									 MoveTemp(Response.Response).Release(), nullptr };
		Worker_Connection_SendCommandResponse(Connection.Get(), Response.RequestId, &r);
	}

	for (auto& Failure : Messages->EntityCommandFailures)
	{
		Worker_Connection_SendCommandFailure(Connection.Get(), Failure.RequestId, TCHAR_TO_UTF8(*Failure.Message));
	}

	for (auto& Log : Messages->Logs)
	{
		FTCHARToUTF8 LoggerName(*Log.LoggerName.ToString());
		FTCHARToUTF8 LogString(*Log.Message);
		Worker_LogMessage L = { static_cast<uint8>(Log.Level), LoggerName.Get(), LogString.Get() };
		Worker_Connection_SendLogMessage(Connection.Get(), &L);
	}

	for (auto& Metrics : Messages->Metrics)
	{
		Metrics.SendToConnection(Connection.Get());
	}
}

const FString& SpatialOSConnectionHandler::GetWorkerId() const
{
	return WorkerId;
}

const TArray<FString>& SpatialOSConnectionHandler::GetWorkerAttributes() const
{
	return WorkerAttributes;
}

void SpatialOSConnectionHandler::ConnectionDeleter::operator()(Worker_Connection* ConnectionToDelete) const noexcept
{
	if (ConnectionToDelete != nullptr)
	{
		Worker_Connection_Destroy(ConnectionToDelete);
	}
}

} // namespace SpatialGDK
