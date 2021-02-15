// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/CrossServerRPCSender.h"

#include "Interop/Connection/SpatialEventTracer.h"
#include "Interop/Connection/SpatialTraceEventBuilder.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Utils/SpatialMetrics.h"

namespace SpatialGDK
{
CrossServerRPCSender::CrossServerRPCSender(ViewCoordinator& InCoordinator, USpatialMetrics* InSpatialMetrics,
										   SpatialEventTracer* EventTracer)
	: Coordinator(&InCoordinator)
	, SpatialMetrics(InSpatialMetrics)
	, EventTracer(EventTracer)
{
}

void CrossServerRPCSender::SendCommand(const FUnrealObjectRef InTargetObjectRef, UObject* TargetObject, UFunction* Function,
									   RPCPayload&& InPayload, FRPCInfo Info) const
{
	if (Function == nullptr || TargetObject == nullptr || InTargetObjectRef.Entity == SpatialConstants::INVALID_ENTITY_ID
		|| Info.Type != ERPCType::CrossServer)
	{
		return;
	}

	CommandRequest CommandRequest(SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID,
								  SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID);

	uint64 UniqueRPCId = FMath::RandHelper(INT_MAX);
	RPCPayload::WriteToSchemaObject(CommandRequest.GetRequestObject(), InTargetObjectRef.Offset, Info.Index, UniqueRPCId,
									InPayload.PayloadData.GetData(), InPayload.PayloadData.Num());

	FSpatialGDKSpanId SpanId;
	if (EventTracer)
	{
		SpanId = EventTracer->TraceEvent(
			FSpatialTraceEventBuilder::CreateSendCrossServerRPC(
				TargetObject, Function, EventTraceUniqueId::GenerateForCrossServerRPC(InTargetObjectRef.Entity, UniqueRPCId)),
			/* Causes */ EventTracer->GetFromStack().GetConstId(), /* NumCauses */ 1);
	}

	if (Function->HasAnyFunctionFlags(FUNC_NetReliable))
	{
		Coordinator->SendEntityCommandRequest(InTargetObjectRef.Entity, MoveTemp(CommandRequest), RETRY_MAX_TIMES, SpanId);
	}
	else
	{
		Coordinator->SendEntityCommandRequest(InTargetObjectRef.Entity, MoveTemp(CommandRequest), TOptional<uint32>(), SpanId);
	}

#if !UE_BUILD_SHIPPING
	SpatialMetrics->TrackSentRPC(Function, ERPCType::CrossServer, InPayload.PayloadData.Num());
#endif // !UE_BUILD_SHIPPING
}
} // namespace SpatialGDK
