// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "Connection/SpatialOSWorkerInterface.h"

class USpatialWorkerConnection;

namespace SpatialGDK
{
class FSubView;

template <decltype(Worker_Op::op_type) WorkerOpType, class TWorkerOpType, TWorkerOpType Worker_Op_Union::*OpTypeUnionPtr, class TCallable>
class SpatialRequestHandler
{
public:
	void ProcessOps(const TArray<Worker_Op>& Ops)
	{
		for (const Worker_Op& Op : Ops)
		{
			if (Op.op_type == WorkerOpType)
			{
				const TWorkerOpType& TypedOp = Op.op.*OpTypeUnionPtr;
				const Worker_RequestId& RequestId = TypedOp.request_id;
				const auto RequestStatus = TypedOp.status_code;

				TCallable CallableToCall;
				if (Handlers.RemoveAndCopyValue(RequestId, CallableToCall))
				{
					if (ensure(CallableToCall.IsBound()))
					{
						CallableToCall.Execute(TypedOp);
					}
				}
			}
		}
	}

	void AddRequest(Worker_RequestId RequestId, const TCallable& Callable) { Handlers.Add(RequestId, Callable); }

private:
	TMap<Worker_RequestId_Key, TCallable> Handlers;
};

class ReserveEntityHandler : public SpatialRequestHandler<WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE, Worker_ReserveEntityIdsResponseOp,
														  &Worker_Op_Union::reserve_entity_ids_response, ReserveEntityIDsDelegate>
{
};

class ClaimPartitionHandler
{
public:
	ClaimPartitionHandler(SpatialOSWorkerInterface& InWorkerInterface);

	void ClaimPartition(Worker_EntityId SystemEntityId, Worker_PartitionId PartitionToClaim);

	void ProcessOps(const TArray<Worker_Op>& Ops);

private:
	TMap<Worker_RequestId_Key, Worker_PartitionId> ClaimPartitionRequestIds;

	SpatialOSWorkerInterface& WorkerInterface;
};
} // namespace SpatialGDK