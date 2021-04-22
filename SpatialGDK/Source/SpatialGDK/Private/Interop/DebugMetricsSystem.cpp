// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/DebugMetricsSystem.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Interop/Connection/SpatialWorkerConnection.h"

DEFINE_LOG_CATEGORY_STATIC(LogSpatialDebugMetrics, Log, All);

namespace SpatialGDK
{
DebugMetricsSystem::DebugMetricsSystem(USpatialNetDriver& InNetDriver)
	: Connection(*InNetDriver.Connection)
	, SpatialMetrics(*InNetDriver.SpatialMetrics)
	, EventTracer(InNetDriver.Connection->GetEventTracer())
{
}

void DebugMetricsSystem::ProcessOps(const TArray<Worker_Op>& Ops) const
{
#if !UE_BUILD_SHIPPING
	for (const Worker_Op& Op : Ops)
	{
		if (Op.op_type == WORKER_OP_TYPE_METRICS)
		{
			SpatialMetrics.HandleWorkerMetrics(Op);
		}
		else if (Op.op_type == WORKER_OP_TYPE_COMMAND_REQUEST)
		{
			const Worker_CommandRequestOp& CommandRequest = Op.op.command_request;

			const Worker_RequestId RequestId = CommandRequest.request_id;
			const Worker_ComponentId ComponentId = CommandRequest.request.component_id;
			const Worker_CommandIndex CommandIndex = CommandRequest.request.command_index;
			const Worker_EntityId EntityId = CommandRequest.entity_id;

			if (ComponentId == SpatialConstants::DEBUG_METRICS_COMPONENT_ID)
			{
				switch (CommandIndex)
				{
				case SpatialConstants::DEBUG_METRICS_START_RPC_METRICS_ID:
					SpatialMetrics.OnStartRPCMetricsCommand();
					break;
				case SpatialConstants::DEBUG_METRICS_STOP_RPC_METRICS_ID:
					SpatialMetrics.OnStopRPCMetricsCommand();
					break;
				case SpatialConstants::DEBUG_METRICS_MODIFY_SETTINGS_ID:
				{
					Schema_Object* Payload = Schema_GetCommandRequestObject(CommandRequest.request.schema_type);
					SpatialMetrics.OnModifySettingCommand(Payload);
					break;
				}
				default:
					UE_LOG(LogSpatialDebugMetrics, Error, TEXT("Unknown command index for DebugMetrics component: %d, entity: %lld"),
						   CommandIndex, EntityId);
					break;
				}

				{
					Worker_CommandResponse Response = {};
					Response.component_id = ComponentId;
					Response.command_index = CommandIndex;
					Response.schema_type = Schema_CreateCommandResponse();

					const FSpatialGDKSpanId CauseSpanId(Op.span_id);
					FSpatialGDKSpanId SpanId;

					if (EventTracer != nullptr)
					{
						SpanId = EventTracer->TraceEvent(FSpatialTraceEventName::SendCommandResponseEventName, "", CauseSpanId.GetConstId(),
														 1, [RequestId](FSpatialTraceEventDataBuilder& EventBuilder) {
															 EventBuilder.AddRequestId(RequestId);
															 EventBuilder.AddKeyValue("Success", true);
														 });
					}

					Connection.SendCommandResponse(RequestId, &Response, SpanId);
				}
			}
		}
	}
#endif // !UE_BUILD_SHIPPING
}
} // namespace SpatialGDK
