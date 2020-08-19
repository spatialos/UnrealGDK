// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/CommandRetryHandler.h"

#include "Algo/Transform.h"
#include "Misc/Optional.h"

namespace SpatialGDK
{
class FDeleteEntityRetryHandler : public TCommandRetryHandler<FDeleteEntityRetryHandler, Worker_EntityId>
{
public:
	explicit FDeleteEntityRetryHandler(WorkerView* Worker)
		: TCommandRetryHandler(Worker)
	{
	}

private:
	static bool CanHandleOp(const Worker_Op& Op) { return Op.op_type == WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE; }

	static Worker_RequestId GetRequestId(const Worker_Op& Op)
	{
		check(Op.op_type == WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE);
		return Op.op.delete_entity_response.request_id;
	}

	static bool CanRetry(const Worker_Op& Op, FRetryData& RetryData)
	{
		RetryData = RetryData.Advance();
		return Op.op.delete_entity_response.status_code == WORKER_STATUS_CODE_TIMEOUT;
	}

	void SendCommandRequest(Worker_RequestId RequestId, Worker_EntityId EntityId, uint32 TimeoutMillis)
	{
		Worker->SendDeleteEntityRequest(DeleteEntityRequest{ RequestId, EntityId, TimeoutMillis });
	}

	friend TCommandRetryHandler<FDeleteEntityRetryHandler, Worker_EntityId>;
};

struct FCreateEntityData
{
	TArray<ComponentData> Components;
	TOptional<Worker_EntityId> EntityId;
};

class FCreateEntityRetryHandler : public TCommandRetryHandler<FCreateEntityRetryHandler, FCreateEntityData>
{
public:
	explicit FCreateEntityRetryHandler(WorkerView* Worker)
		: TCommandRetryHandler(Worker)
	{
	}

private:
	static bool CanHandleOp(const Worker_Op& Op) { return Op.op_type == WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE; }

	static Worker_RequestId GetRequestId(const Worker_Op& Op)
	{
		check(Op.op_type == WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE);
		return Op.op.create_entity_response.request_id;
	}

	static bool CanRetry(const Worker_Op& Op, FRetryData& RetryData)
	{
		RetryData = RetryData.Advance();
		return Op.op.create_entity_response.status_code == WORKER_STATUS_CODE_TIMEOUT;
	}

	void SendCommandRequest(Worker_RequestId RequestId, const FCreateEntityData& Data, uint32 TimeoutMillis)
	{
		TArray<ComponentData> ComponentsCopy;
		ComponentsCopy.Reserve(Data.Components.Num());
		Algo::Transform(Data.Components, ComponentsCopy, [](const ComponentData& Component) {
			return Component.DeepCopy();
		});

		Worker->SendCreateEntityRequest(CreateEntityRequest{ RequestId, MoveTemp(ComponentsCopy), Data.EntityId, TimeoutMillis });
	}

	void SendCommandRequest(Worker_RequestId RequestId, FCreateEntityData&& Data, uint32 TimeoutMillis)
	{
		Worker->SendCreateEntityRequest(CreateEntityRequest{ RequestId, MoveTemp(Data.Components), Data.EntityId, TimeoutMillis });
	}

	friend TCommandRetryHandler<FCreateEntityRetryHandler, FCreateEntityData>;
};

class FReserveEntityIdsRetryHandler : public TCommandRetryHandler<FReserveEntityIdsRetryHandler, uint32>
{
public:
	explicit FReserveEntityIdsRetryHandler(WorkerView* Worker)
		: TCommandRetryHandler(Worker)
	{
	}

private:
	static bool CanHandleOp(const Worker_Op& Op) { return Op.op_type == WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE; }

	static Worker_RequestId GetRequestId(const Worker_Op& Op)
	{
		check(Op.op_type == WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE);
		return Op.op.reserve_entity_ids_response.request_id;
	}

	static bool CanRetry(const Worker_Op& Op, FRetryData& RetryData)
	{
		RetryData = RetryData.Advance();
		return Op.op.reserve_entity_ids_response.status_code == WORKER_STATUS_CODE_TIMEOUT;
	}

	void SendCommandRequest(Worker_RequestId RequestId, uint32 NumberOfIds, uint32 TimeoutMillis)
	{
		Worker->SendReserveEntityIdsRequest(ReserveEntityIdsRequest{ RequestId, NumberOfIds, TimeoutMillis });
	}

	friend TCommandRetryHandler<FReserveEntityIdsRetryHandler, uint32>;
};

class FEntityQueryRetryHandler : public TCommandRetryHandler<FEntityQueryRetryHandler, EntityQuery>
{
public:
	explicit FEntityQueryRetryHandler(WorkerView* Worker)
		: TCommandRetryHandler(Worker)
	{
	}

private:
	static bool CanHandleOp(const Worker_Op& Op) { return Op.op_type == WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE; }

	static Worker_RequestId GetRequestId(const Worker_Op& Op)
	{
		check(Op.op_type == WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE);
		return Op.op.entity_query_response.request_id;
	}

	static bool CanRetry(const Worker_Op& Op, FRetryData& RetryData)
	{
		RetryData = RetryData.Advance();
		return Op.op.entity_query_response.status_code == WORKER_STATUS_CODE_TIMEOUT;
	}

	void SendCommandRequest(Worker_RequestId RequestId, const EntityQuery& Query, uint32 TimeoutMillis)
	{
		Worker->SendEntityQueryRequest(EntityQueryRequest{ RequestId, EntityQuery(Query.GetWorkerQuery()), TimeoutMillis });
	}

	void SendCommandRequest(Worker_RequestId RequestId, EntityQuery&& Query, uint32 TimeoutMillis)
	{
		Worker->SendEntityQueryRequest(EntityQueryRequest{ RequestId, MoveTemp(Query), TimeoutMillis });
	}

	friend TCommandRetryHandler<FEntityQueryRetryHandler, EntityQuery>;
};

struct FEntityCommandData
{
	Worker_EntityId EntityId;
	CommandRequest Request;
};

class FEntityCommandRetryHandler : public TCommandRetryHandler<FEntityCommandRetryHandler, FEntityCommandData>
{
public:
	explicit FEntityCommandRetryHandler(WorkerView* Worker)
		: TCommandRetryHandler(Worker)
	{
	}

private:
	static bool CanHandleOp(const Worker_Op& Op) { return Op.op_type == WORKER_OP_TYPE_COMMAND_RESPONSE; }

	static Worker_RequestId GetRequestId(const Worker_Op& Op)
	{
		check(Op.op_type == WORKER_OP_TYPE_COMMAND_RESPONSE);
		return Op.op.command_response.request_id;
	}

	static bool CanRetry(const Worker_Op& Op, FRetryData& RetryData)
	{
		switch (static_cast<Worker_StatusCode>(Op.op.command_response.status_code))
		{
		case WORKER_STATUS_CODE_TIMEOUT:
			RetryData = RetryData.Advance();
			return true;
		case WORKER_STATUS_CODE_AUTHORITY_LOST:
			// Don't increase retry timer or reduce the number of retires.
			return true;
		default:
			return false;
		}
	}

	void SendCommandRequest(Worker_RequestId RequestId, const FEntityCommandData& Query, uint32 TimeoutMillis)
	{
		Worker->SendEntityCommandRequest(EntityCommandRequest{ Query.EntityId, RequestId, Query.Request.DeepCopy(), TimeoutMillis });
	}

	void SendCommandRequest(Worker_RequestId RequestId, FEntityCommandData&& Query, uint32 TimeoutMillis)
	{
		Worker->SendEntityCommandRequest(EntityCommandRequest{ Query.EntityId, RequestId, MoveTemp(Query.Request), TimeoutMillis });
	}

	friend TCommandRetryHandler<FEntityCommandRetryHandler, FEntityCommandData>;
};
} // namespace SpatialGDK
