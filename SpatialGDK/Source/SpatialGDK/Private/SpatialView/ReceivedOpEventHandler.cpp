// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/ReceivedOpEventHandler.h"

#include "Interop/Connection/SpatialTraceEventBuilder.h"

namespace SpatialGDK
{
FReceivedOpEventHandler::FReceivedOpEventHandler(TSharedPtr<SpatialEventTracer> EventTracer)
	: EventTracer(MoveTemp(EventTracer))
{
}

void FReceivedOpEventHandler::ProcessOpLists(const OpList& Ops)
{
	if (EventTracer == nullptr || !EventTracer->IsEnabled())
	{
		return;
	}

	for (uint32 i = 0; i < Ops.Count; ++i)
	{
		Worker_Op& Op = Ops.Ops[i];

		TOptional<Trace_SpanId> SpanId = EventTracer->CreateSpan(&Op.span_id, 1);

		switch (static_cast<Worker_OpType>(Op.op_type))
		{
		case WORKER_OP_TYPE_ADD_ENTITY:
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::ReceiveCreateEntity(Op.op.add_entity.entity_id),
									EventTracer->CreateSpan(&Op.span_id, 1));
			break;
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			EventTracer->TraceEvent(FSpatialTraceEventBuilder::ReceiveRemoveEntity(Op.op.remove_entity.entity_id),
									EventTracer->CreateSpan(&Op.span_id, 1));
			break;
		case WORKER_OP_TYPE_ADD_COMPONENT:
			EventTracer->AddComponent(Op.op.add_component.entity_id, Op.op.add_component.data.component_id, Op.span_id);
			break;
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			EventTracer->RemoveComponent(Op.op.remove_component.entity_id, Op.op.remove_component.component_id);
			break;
		case WORKER_OP_TYPE_AUTHORITY_CHANGE:
			EventTracer->TraceEvent(
				FSpatialTraceEventBuilder::AuthorityChange(Op.op.authority_change.entity_id, Op.op.authority_change.component_id,
														   static_cast<Worker_Authority>(Op.op.authority_change.authority)),
				EventTracer->CreateSpan(&Op.span_id, 1));
			break;
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			EventTracer->UpdateComponent(Op.op.component_update.entity_id, Op.op.component_update.update.component_id, Op.span_id);
			break;
		default:
			break;
		}
	}
}

} // namespace SpatialGDK
