// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

DECLARE_CYCLE_STAT(TEXT("CreateEntityHandler"), STAT_CreateEntityHandler, STATGROUP_SpatialNet);

using CreateEntityDelegate = TFunction<void(const Worker_CreateEntityResponseOp&)>;

namespace SpatialGDK
{
class CreateEntityHandler
{
	DECLARE_LOG_CATEGORY_CLASS(LogCreateEntityHandler, Log, All);

public:
	void AddRequest(Worker_RequestId RequestId, CreateEntityDelegate&& Handler)
	{
		if (!ensureAlwaysMsgf(Handler, TEXT("Failed to add create entity requested handler. Handler delegate was unbound")))
		{
			return;
		}
		Handlers.Emplace(RequestId, MoveTemp(Handler));
	}

	void ProcessOps(const TArray<Worker_Op>& Ops)
	{
		SCOPE_CYCLE_COUNTER(STAT_CreateEntityHandler);

		for (const Worker_Op& Op : Ops)
		{
			if (Op.op_type == WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE)
			{
				const Worker_CreateEntityResponseOp& EntityIdsOp = Op.op.create_entity_response;

				if (EntityIdsOp.status_code != WORKER_STATUS_CODE_SUCCESS)
				{
					UE_LOG(LogCreateEntityHandler, Warning, TEXT("CreateEntity request failed: request id: %d, message: %s"),
						   EntityIdsOp.request_id, UTF8_TO_TCHAR(EntityIdsOp.message));
					return;
				}

				UE_LOG(LogCreateEntityHandler, Verbose, TEXT("CreateEntity request succeeded: request id: %d, message: %s"),
					   EntityIdsOp.request_id, UTF8_TO_TCHAR(EntityIdsOp.message));

				const Worker_RequestId RequestId = EntityIdsOp.request_id;
				CreateEntityDelegate Handler;
				const bool bHasHandler = Handlers.RemoveAndCopyValue(RequestId, Handler);
				if (bHasHandler)
				{
					if (ensure(Handler))
					{
						Handler(EntityIdsOp);
					}
				}
			}
		}
	}

	int GetPendingRequestsCount() const { return Handlers.Num(); }

private:
	TMap<Worker_RequestId_Key, CreateEntityDelegate> Handlers;
};
} // namespace SpatialGDK
