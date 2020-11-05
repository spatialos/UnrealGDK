// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/CrossServerRPCSender.h"

#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Utils/SpatialMetrics.h"

using namespace SpatialGDK;

CrossServerRPCSender::CrossServerRPCSender(ViewCoordinator& InCoordinator, USpatialMetrics* InSpatialMetrics)
	: Coordinator(InCoordinator)
	, SpatialMetrics(InSpatialMetrics)
{
}

void CrossServerRPCSender::SendCommand(const FUnrealObjectRef& InTargetObjectRef, UObject* TargetObject, UFunction* Function,
									   RPCPayload&& InPayload, FRPCInfo Info, const TOptional<Trace_SpanId>& SpanId) const
{
	if (Function == nullptr || TargetObject == nullptr || InTargetObjectRef.Entity == SpatialConstants::INVALID_ENTITY_ID
		|| Info.Type != ERPCType::CrossServer)
	{
		return;
	}

	auto SchemaType = Schema_CreateCommandRequest();
	Schema_Object* RequestObject = Schema_GetCommandRequestObject(SchemaType);
	RPCPayload::WriteToSchemaObject(RequestObject, InTargetObjectRef.Offset, Info.Index, Function->GetUniqueID(), InPayload.PayloadData.GetData(),
									InPayload.PayloadData.Num());
	CommandRequest CommandRequest(OwningCommandRequestPtr(SchemaType), SpatialConstants::SERVER_TO_SERVER_COMMAND_ENDPOINT_COMPONENT_ID,
								  SpatialConstants::UNREAL_RPC_ENDPOINT_COMMAND_ID);

	if (Function->HasAnyFunctionFlags(FUNC_NetReliable))
	{
		Coordinator.SendEntityCommandRequest(InTargetObjectRef.Entity, MoveTemp(CommandRequest), RETRY_MAX_TIMES, SpanId);
	}
	else
	{
		Coordinator.SendEntityCommandRequest(InTargetObjectRef.Entity, MoveTemp(CommandRequest), TOptional<uint32>(), SpanId);
	}

#if !UE_BUILD_SHIPPING
	SpatialMetrics->TrackSentRPC(Function, ERPCType::CrossServer, InPayload.PayloadData.Num());
#endif // !UE_BUILD_SHIPPING
}
