// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/CrossServerRPCHandler.h"

#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "SpatialGDKLLM.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogCrossServerRPCHandler);

namespace SpatialGDK
{
CrossServerRPCHandler::CrossServerRPCHandler(ViewCoordinator& InCoordinator, TUniquePtr<RPCExecutorInterface> InRPCExecutor,
											 SpatialEventTracer* InEventTracer)
	: Coordinator(&InCoordinator)
	, RPCExecutor(MoveTemp(InRPCExecutor))
	, EventTracer(InEventTracer)
{
}

void CrossServerRPCHandler::ProcessMessages(const TArray<Worker_Op>& WorkerMessages, float DeltaTime)
{
	LLM_PLATFORM_SCOPE_SPATIAL(ELLMTagSpatialGDK::CrossServerRPCHandler);
	CurrentTime += DeltaTime;

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

	while (RPCsToDelete.Num() > 0 && RPCsToDelete.HeapTop().Key < CurrentTime)
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

			RPCGuidsInFlight.Remove(Command.Payload.Id.GetValue());
			RPCsToDelete.HeapPush(TTuple<double, uint32>(CurrentTime + CrossServerRPCGuidTimeout, Command.Payload.Id.GetValue()));
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
	TOptional<FCrossServerRPCParams> Params = RPCExecutor->TryRetrieveCrossServerRPCParams(Op);

	FSpatialGDKSpanId SpanId;
	if (EventTracer)
	{
		if (ensureMsgf(Params->Payload.Id.IsSet(), TEXT("Cross-server RPCs are expected to have a payload ID for event tracing.")))
		{
			SpanId = EventTracer->TraceEvent(
				FSpatialTraceEventBuilder::CreateReceiveCrossServerRPC(
					EventTraceUniqueId::GenerateForCrossServerRPC(CommandOp.entity_id, Params->Payload.Id.GetValue())),
				/* Causes */ EventTracer->GetAndConsumeSpanForRequestId(Op.op.command_request.request_id).GetConstId(), /* NumCauses */ 1);
		}
	}

	if (!Params.IsSet())
	{
		Coordinator->SendEntityCommandFailure(CommandOp.request_id, TEXT("Failed to parse cross server RPC"), SpanId);
		return;
	}

	Params->SpanId = SpanId;

	if (RPCGuidsInFlight.Contains(Params.GetValue().Payload.Id.GetValue())
		|| RPCsToDelete.ContainsByPredicate([&](TTuple<double, uint32> Result) {
			   return Params.GetValue().Payload.Id == Result.Value;
		   }))
	{
		// This RPC is already in flight. No need to store it again.
		UE_LOG(LogCrossServerRPCHandler, Log, TEXT("RPC is already in flight."));
		return;
	}

	if (!QueuedCrossServerRPCs.Contains(CommandOp.entity_id))
	{
		// No Command Requests of this type queued so far. Let's try to process it:
		if (TryExecuteCrossServerRPC(Params.GetValue()))
		{
			RPCsToDelete.HeapPush(TTuple<double, uint32>(CurrentTime + CrossServerRPCGuidTimeout, Params.GetValue().Payload.Id.GetValue()));
			return;
		}
	}

	// Unable to process command request. Let's queue it up:
	if (!QueuedCrossServerRPCs.Contains(CommandOp.entity_id))
	{
		QueuedCrossServerRPCs.Add(CommandOp.entity_id, TArray<FCrossServerRPCParams>());
	}

	RPCGuidsInFlight.Add(Params.GetValue().Payload.Id.GetValue());
	QueuedCrossServerRPCs[CommandOp.entity_id].Add(MoveTemp(Params.GetValue()));
}

bool CrossServerRPCHandler::TryExecuteCrossServerRPC(const FCrossServerRPCParams& Params) const
{
	bool bResult = RPCExecutor->ExecuteCommand(Params);
	if (bResult)
	{
		Coordinator->SendEntityCommandResponse(Params.RequestId,
											   CommandResponse(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID,
															   SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID),
											   Params.SpanId);
	}

	return bResult;
}

void CrossServerRPCHandler::DropQueueForEntity(const Worker_EntityId_Key EntityId)
{
	TArray<FCrossServerRPCParams>* Params = QueuedCrossServerRPCs.Find(EntityId);
	if (Params == nullptr)
	{
		return;
	}

	for (const auto& Command : *Params)
	{
		RPCGuidsInFlight.Remove(Command.Payload.Id.GetValue());
	}

	QueuedCrossServerRPCs.Remove(EntityId);
}

int32 CrossServerRPCHandler::GetRPCGuidsInFlightCount() const
{
	return RPCGuidsInFlight.Num();
}

int32 CrossServerRPCHandler::GetRPCsToDeleteCount() const
{
	return RPCsToDelete.Num();
}
} // namespace SpatialGDK
