// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "Connection/SpatialOSWorkerInterface.h"

DECLARE_CYCLE_STAT(TEXT("ReserveEntityHandler"), STAT_ReserveEntityHandler, STATGROUP_SpatialNet);

using ReserveEntityIDsDelegate = TFunction<void(const Worker_ReserveEntityIdsResponseOp&)>;

namespace SpatialGDK
{
class ReserveEntityIdsHandler
{
public:
	void ProcessOps(const TArray<Worker_Op>& Ops)
	{
		SCOPE_CYCLE_COUNTER(STAT_ReserveEntityHandler);

		for (const Worker_Op& Op : Ops)
		{
			if (Op.op_type == WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE)
			{
				const Worker_ReserveEntityIdsResponseOp& TypedOp = Op.op.reserve_entity_ids_response;
				const Worker_RequestId& RequestId = TypedOp.request_id;

				ReserveEntityIDsDelegate CallableToCall;
				if (Handlers.RemoveAndCopyValue(RequestId, CallableToCall))
				{
					if (ensure(CallableToCall))
					{
						CallableToCall(TypedOp);
					}
				}
			}
		}
	}

	void AddRequest(Worker_RequestId RequestId, const ReserveEntityIDsDelegate& Callable) { Handlers.Add(RequestId, Callable); }

private:
	TMap<Worker_RequestId_Key, ReserveEntityIDsDelegate> Handlers;
};
} // namespace SpatialGDK
