// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/CrossServerRPCHandler.h"

#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialGDKSettings.h"

using namespace SpatialGDK;

CrossServerRPCHandler::CrossServerRPCHandler(ViewCoordinator& InCoordinator, TUniquePtr<RPCExecutorInterface> InRPCExecutor)
	: Coordinator(InCoordinator)
	, RPCExecutor(MoveTemp(InRPCExecutor))
{
	CommandRetryTime = GetDefault<USpatialGDKSettings>()->QueuedIncomingRPCRetryTime;
}

void CrossServerRPCHandler::ProcessOps(const float TimeAdvancedS, const TArray<Worker_Op>& WorkerMessages)
{
	for (auto& Op : WorkerMessages)
	{
		if (Op.op_type == WORKER_OP_TYPE_COMMAND_REQUEST
			&& Op.op.command_request.request.command_index == SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID
			&& Op.op.command_request.request.component_id == SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID)
		{
			HandleWorkerOp(Op);
		}
	}

	TimeElapsedS += TimeAdvancedS;
	if (CommandRetryTime < TimeElapsedS)
	{
		ProcessPendingCommandOps();
		TimeElapsedS = 0.0;
	}
}

void CrossServerRPCHandler::ProcessPendingCommandOps()
{
	TArray<Worker_EntityId> EmptyRPCQueues;
	for (auto& CrossServerRPCs : QueuedCrossServerRPCs)
	{
		int32 ProcessedRPCs = 0;
		for (auto& Command : CrossServerRPCs.Value)
		{
			if (!TryExecuteCommandRequest(Command))
			{
				break;
			}

			RPCGuidsInFlight.Remove(Command.Payload.UniqueId);
			++ProcessedRPCs;
		}

		CrossServerRPCs.Value.RemoveAt(0, ProcessedRPCs);
		if (CrossServerRPCs.Value.Num() == 0)
		{
			EmptyRPCQueues.Add(CrossServerRPCs.Key);
		}
	}
	for (auto EntityId : EmptyRPCQueues)
	{
		QueuedCrossServerRPCs.Remove(EntityId);
	}
}

void CrossServerRPCHandler::HandleWorkerOp(const Worker_Op& Op)
{
	const Worker_CommandRequestOp& CommandOp = Op.op.command_request;
	FCrossServerRPCParams Params = RPCExecutor->TryRetrieveCrossServerRPCParams(Op);
	if (Params.RequestId == -1)
	{
		Coordinator.SendEntityCommandFailure(CommandOp.request_id, "Failed to parse cross server RPC", Op.span_id);
		return;
	}

	if (RPCGuidsInFlight.Contains(Params.Payload.UniqueId))
	{
		// This RPC is already in flight. No need to store it again.
		return;
	}

	if (!QueuedCrossServerRPCs.Contains(CommandOp.entity_id))
	{
		// No Command Requests of this type queued so far. Let's try to process it:
		if (TryExecuteCommandRequest(Params))
		{
			return;
		}
	}

	// Unable to process command request. Let's queue it up:
	if (!QueuedCrossServerRPCs.Contains(CommandOp.entity_id))
	{
		QueuedCrossServerRPCs.Add(CommandOp.entity_id, TArray<FCrossServerRPCParams>());
	}

	QueuedCrossServerRPCs[CommandOp.entity_id].Add(MoveTemp(Params));
}

bool CrossServerRPCHandler::TryExecuteCommandRequest(const FCrossServerRPCParams& Params)
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
	for (auto& Command : QueuedCrossServerRPCs[EntityId])
	{
		RPCGuidsInFlight.Remove(Command.Payload.UniqueId);
	}

	QueuedCrossServerRPCs.Remove(EntityId);
}
