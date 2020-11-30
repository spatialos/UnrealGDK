// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/CrossServerRPCHandler.h"

#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogCrossServerRPCHandler);

namespace SpatialGDK
{
CrossServerRPCHandler::CrossServerRPCHandler(ViewCoordinator& InCoordinator, TUniquePtr<RPCExecutorInterface> InRPCExecutor)
	: Coordinator(InCoordinator)
	, RPCExecutor(MoveTemp(InRPCExecutor))
{
}

void CrossServerRPCHandler::ProcessOps(const TArray<Worker_Op>& WorkerMessages)
{
	for (const auto& Op : WorkerMessages)
	{
		if (Op.op_type == WORKER_OP_TYPE_COMMAND_REQUEST
			&& Op.op.command_request.request.command_index == SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID
			&& Op.op.command_request.request.component_id == SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID)
		{
			HandleWorkerOp(Op);
		}
	}

	ProcessPendingCrossServerRPCs();

	while (RPCsToDelete.Num() > 0 && RPCsToDelete.HeapTop().Key < FDateTime::Now())
	{
		RPCsToDelete.HeapPopDiscard();
	}
}

void CrossServerRPCHandler::ProcessPendingCrossServerRPCs()
{
	for (auto CrossServerRPCs = QueuedCrossServerRPCs.CreateIterator(); CrossServerRPCs; ++CrossServerRPCs)
	{
		int32 ProcessedRPCs = 0;
		for (const auto& Command : CrossServerRPCs.Value())
		{
			if (!TryExecuteCrossServerRPC(Command))
			{
				break;
			}

			RPCGuidsInFlight.Remove(Command.Payload.UniqueId);
			RPCsToDelete.HeapPush(TTuple<FDateTime, uint32>(FDateTime::Now() + Command.TimeoutMillis, Command.Payload.UniqueId));
			++ProcessedRPCs;
		}

		CrossServerRPCs.Value().RemoveAt(0, ProcessedRPCs);
		if (CrossServerRPCs.Value().Num() == 0)
		{
			CrossServerRPCs.RemoveCurrent();
		}
	}
}

const TMap<Worker_EntityId_Key, TArray<FCrossServerRPCParams>>& CrossServerRPCHandler::GetQueuedCrossServerRPCs() const
{
	return QueuedCrossServerRPCs;
}

void CrossServerRPCHandler::HandleWorkerOp(const Worker_Op& Op)
{
	const Worker_CommandRequestOp& CommandOp = Op.op.command_request;
	FCrossServerRPCParams Params = RPCExecutor->TryRetrieveCrossServerRPCParams(Op);

	if (Params.RequestId == -1)
	{
		Coordinator.SendEntityCommandFailure(CommandOp.request_id, TEXT("Failed to parse cross server RPC"), FSpatialGDKSpanId(Op.span_id));
		return;
	}

	if (RPCGuidsInFlight.Contains(Params.Payload.UniqueId) || RPCsToDelete.ContainsByPredicate([&](TTuple<FDateTime, uint32> Result) {
			return Params.Payload.UniqueId == Result.Value;
		}))
	{
		// This RPC is already in flight. No need to store it again.
		UE_LOG(LogCrossServerRPCHandler, Warning, TEXT("RPC is already in flight."));
		return;
	}

	if (!QueuedCrossServerRPCs.Contains(CommandOp.entity_id))
	{
		// No Command Requests of this type queued so far. Let's try to process it:
		if (TryExecuteCrossServerRPC(Params))
		{
			return;
		}
	}

	// Unable to process command request. Let's queue it up:
	if (!QueuedCrossServerRPCs.Contains(CommandOp.entity_id))
	{
		QueuedCrossServerRPCs.Add(CommandOp.entity_id, TArray<FCrossServerRPCParams>());
	}

	RPCGuidsInFlight.Add(Params.Payload.UniqueId);
	QueuedCrossServerRPCs[CommandOp.entity_id].Add(MoveTemp(Params));
}

bool CrossServerRPCHandler::TryExecuteCrossServerRPC(const FCrossServerRPCParams& Params) const
{
	if (RPCExecutor->ExecuteCommand(Params))
	{
		Coordinator.SendEntityCommandResponse(Params.RequestId,
											  CommandResponse(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID,
															  SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID),
											  Params.SpanId);
		return true;
	}

	return false;
}

void CrossServerRPCHandler::DropQueueForEntity(const Worker_EntityId_Key EntityId)
{
	for (const auto& Command : QueuedCrossServerRPCs[EntityId])
	{
		RPCGuidsInFlight.Remove(Command.Payload.UniqueId);
	}

	QueuedCrossServerRPCs.Remove(EntityId);
}
} // namespace SpatialGDK
