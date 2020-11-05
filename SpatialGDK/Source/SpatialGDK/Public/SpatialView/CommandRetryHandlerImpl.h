// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/CommandRetryHandler.h"

#include "Algo/Transform.h"
#include "Misc/Optional.h"

namespace SpatialGDK
{
struct FDeleteEntityRetryHandlerImpl
{
	struct CommandData
	{
		Worker_EntityId EntityId;
		TOptional<Trace_SpanId> SpanId;
	};

	static bool CanHandleOp(const Worker_Op& Op) { return Op.op_type == WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE; }

	static Worker_RequestId& GetRequestId(Worker_Op& Op)
	{
		check(CanHandleOp(Op));
		return Op.op.delete_entity_response.request_id;
	}

	static void UpdateRetries(const Worker_Op& Op, FRetryData& RetryData)
	{
		check(CanHandleOp(Op));
		if (Op.op.delete_entity_response.status_code == WORKER_STATUS_CODE_TIMEOUT)
		{
			RetryData.RetryAndBackOff();
		}
		else
		{
			RetryData.StopRetries();
		}
	}

	static void SendCommandRequest(Worker_RequestId RequestId, const CommandData& Data, uint32 TimeoutMillis, WorkerView& View)
	{
		View.SendDeleteEntityRequest(DeleteEntityRequest{ RequestId, Data.EntityId, TimeoutMillis, Data.SpanId });
	}

	static void SendCommandRequest(Worker_RequestId RequestId, CommandData&& Data, uint32 TimeoutMillis, WorkerView& View)
	{
		View.SendDeleteEntityRequest(DeleteEntityRequest{ RequestId, Data.EntityId, TimeoutMillis, Data.SpanId });
	}
};

struct FCreateEntityRetryHandlerImpl
{
	struct CommandData
	{
		TArray<ComponentData> Components;
		TOptional<Worker_EntityId> EntityId;
		TOptional<Trace_SpanId> SpanId;
	};

	static bool CanHandleOp(const Worker_Op& Op) { return Op.op_type == WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE; }

	static Worker_RequestId& GetRequestId(Worker_Op& Op)
	{
		check(CanHandleOp(Op));
		return Op.op.create_entity_response.request_id;
	}

	static void UpdateRetries(const Worker_Op& Op, FRetryData& RetryData)
	{
		check(CanHandleOp(Op));
		if (Op.op.create_entity_response.status_code == WORKER_STATUS_CODE_TIMEOUT)
		{
			RetryData.RetryAndBackOff();
		}
		else
		{
			RetryData.StopRetries();
		}
	}

	static void SendCommandRequest(Worker_RequestId RequestId, const CommandData& Data, uint32 TimeoutMillis, WorkerView& View)
	{
		TArray<ComponentData> ComponentsCopy;
		ComponentsCopy.Reserve(Data.Components.Num());
		Algo::Transform(Data.Components, ComponentsCopy, [](const ComponentData& Component) {
			return Component.DeepCopy();
		});

		View.SendCreateEntityRequest(CreateEntityRequest{ RequestId, MoveTemp(ComponentsCopy), Data.EntityId, TimeoutMillis, Data.SpanId });
	}

	static void SendCommandRequest(Worker_RequestId RequestId, CommandData&& Data, uint32 TimeoutMillis, WorkerView& View)
	{
		View.SendCreateEntityRequest(
			CreateEntityRequest{ RequestId, MoveTemp(Data.Components), Data.EntityId, TimeoutMillis, Data.SpanId });
	}
};

struct FReserveEntityIdsRetryHandlerImpl
{
	using CommandData = uint32;

	static bool CanHandleOp(const Worker_Op& Op) { return Op.op_type == WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE; }

	static Worker_RequestId& GetRequestId(Worker_Op& Op)
	{
		check(CanHandleOp(Op));
		return Op.op.reserve_entity_ids_response.request_id;
	}

	static void UpdateRetries(const Worker_Op& Op, FRetryData& RetryData)
	{
		check(CanHandleOp(Op));
		if (Op.op.reserve_entity_ids_response.status_code == WORKER_STATUS_CODE_TIMEOUT)
		{
			RetryData.RetryAndBackOff();
		}
		else
		{
			RetryData.StopRetries();
		}
	}

	static void SendCommandRequest(Worker_RequestId RequestId, CommandData NumberOfIds, uint32 TimeoutMillis, WorkerView& View)
	{
		View.SendReserveEntityIdsRequest(ReserveEntityIdsRequest{ RequestId, NumberOfIds, TimeoutMillis });
	}
};

struct FEntityQueryRetryHandlerImpl
{
	using CommandData = EntityQuery;

	static bool CanHandleOp(const Worker_Op& Op) { return Op.op_type == WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE; }

	static Worker_RequestId& GetRequestId(Worker_Op& Op)
	{
		check(CanHandleOp(Op));
		return Op.op.entity_query_response.request_id;
	}

	static void UpdateRetries(const Worker_Op& Op, FRetryData& RetryData)
	{
		check(CanHandleOp(Op));
		if (Op.op.entity_query_response.status_code == WORKER_STATUS_CODE_TIMEOUT)
		{
			RetryData.RetryAndBackOff();
		}
		else
		{
			RetryData.StopRetries();
		}
	}

	static void SendCommandRequest(Worker_RequestId RequestId, const CommandData& Query, uint32 TimeoutMillis, WorkerView& View)
	{
		View.SendEntityQueryRequest(EntityQueryRequest{ RequestId, EntityQuery(Query.GetWorkerQuery()), TimeoutMillis });
	}

	static void SendCommandRequest(Worker_RequestId RequestId, CommandData&& Query, uint32 TimeoutMillis, WorkerView& View)
	{
		View.SendEntityQueryRequest(EntityQueryRequest{ RequestId, MoveTemp(Query), TimeoutMillis });
	}
};

struct FEntityCommandRetryHandlerImpl
{
	struct CommandData
	{
		Worker_EntityId EntityId;
		CommandRequest Request;
		TOptional<Trace_SpanId> SpanId;
	};

	static bool CanHandleOp(const Worker_Op& Op) { return Op.op_type == WORKER_OP_TYPE_COMMAND_RESPONSE; }

	static Worker_RequestId& GetRequestId(Worker_Op& Op)
	{
		check(CanHandleOp(Op));
		return Op.op.command_response.request_id;
	}

	static void UpdateRetries(const Worker_Op& Op, FRetryData& RetryData)
	{
		check(CanHandleOp(Op));
		switch (static_cast<Worker_StatusCode>(Op.op.command_response.status_code))
		{
		case WORKER_STATUS_CODE_TIMEOUT:
			RetryData.RetryAndBackOff();
			break;
		case WORKER_STATUS_CODE_AUTHORITY_LOST:
			RetryData.RetryWithoutBackOff();
			break;
		default:
			RetryData.StopRetries();
		}
	}

	static void SendCommandRequest(Worker_RequestId RequestId, const CommandData& Query, uint32 TimeoutMillis, WorkerView& View)
	{
		View.SendEntityCommandRequest(
			EntityCommandRequest{ Query.EntityId, RequestId, Query.Request.DeepCopy(), TimeoutMillis, Query.SpanId });
	}

	static void SendCommandRequest(Worker_RequestId RequestId, CommandData&& Query, uint32 TimeoutMillis, WorkerView& View)
	{
		View.SendEntityCommandRequest(
			EntityCommandRequest{ Query.EntityId, RequestId, MoveTemp(Query.Request), TimeoutMillis, Query.SpanId });
	}
};
} // namespace SpatialGDK
