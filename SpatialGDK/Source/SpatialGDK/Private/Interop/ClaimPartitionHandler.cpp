// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/ClaimPartitionHandler.h"

#include "Schema/StandardLibrary.h"

#include "Interop/Connection/SpatialOSWorkerInterface.h"

#include "SpatialView/CommandRetryHandler.h"

DEFINE_LOG_CATEGORY_STATIC(LogClaimPartitionHandler, Log, All);

namespace SpatialGDK
{
ClaimPartitionHandler::ClaimPartitionHandler(SpatialOSWorkerInterface& InConnection)
	: WorkerInterface(InConnection)
{
}

void ClaimPartitionHandler::ClaimPartition(Worker_EntityId SystemEntityId, Worker_PartitionId PartitionToClaim,
										   FPartitionClaimCallback InCallback)
{
	UE_LOG(LogClaimPartitionHandler, Log,
		   TEXT("SendClaimPartitionRequest. SystemWorkerEntityId: %lld. "
				"PartitionId: %lld"),
		   SystemEntityId, PartitionToClaim);

	Worker_CommandRequest CommandRequest = Worker::CreateClaimPartitionRequest(PartitionToClaim);
	const Worker_RequestId ClaimEntityRequestId =
		WorkerInterface.SendCommandRequest(SystemEntityId, &CommandRequest, RETRY_UNTIL_COMPLETE, {});
	ClaimPartitionRequestIds.Add(ClaimEntityRequestId, { PartitionToClaim, InCallback });
}

void ClaimPartitionHandler::ProcessOps(const TArray<Worker_Op>& Ops)
{
	for (const Worker_Op& Op : Ops)
	{
		if (Op.op_type == WORKER_OP_TYPE_COMMAND_RESPONSE)
		{
			const Worker_CommandResponseOp& CommandResponse = Op.op.command_response;
			FPartitionClaimRequest ClaimedPartitionRequest;
			const bool bIsRequestHandled = ClaimPartitionRequestIds.RemoveAndCopyValue(CommandResponse.request_id, ClaimedPartitionRequest);
			if (bIsRequestHandled)
			{
				if (ClaimedPartitionRequest.Callback)
				{
					Invoke(ClaimedPartitionRequest.Callback);
				}
				ensure(CommandResponse.response.component_id == SpatialConstants::WORKER_COMPONENT_ID);
				UE_CLOG(CommandResponse.status_code != WORKER_STATUS_CODE_SUCCESS, LogClaimPartitionHandler, Error,
						TEXT("Claim partition request for partition %lld finished, SDK returned code %d [%s]"),
						ClaimedPartitionRequest.PartitionId, (int)CommandResponse.status_code, UTF8_TO_TCHAR(CommandResponse.message));
			}
		}
	}
}
} // namespace SpatialGDK
