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
	if (EventTracer == nullptr)
	{
		return;
	}

	EventTracer->BeginOpsForFrame();
	for (uint32 i = 0; i < Ops.Count; ++i)
	{
		Worker_Op& Op = Ops.Ops[i];

		switch (static_cast<Worker_OpType>(Op.op_type))
		{
		case WORKER_OP_TYPE_ADD_ENTITY:
			EventTracer->AddEntity(Op.op.add_entity, FSpatialGDKSpanId(Op.span_id));
			break;
		case WORKER_OP_TYPE_REMOVE_ENTITY:
			EventTracer->RemoveEntity(Op.op.remove_entity, FSpatialGDKSpanId(Op.span_id));
			break;
		case WORKER_OP_TYPE_ADD_COMPONENT:
			EventTracer->AddComponent(Op.op.add_component, FSpatialGDKSpanId(Op.span_id));
			break;
		case WORKER_OP_TYPE_REMOVE_COMPONENT:
			EventTracer->RemoveComponent(Op.op.remove_component, FSpatialGDKSpanId(Op.span_id));
			break;
		case WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE:
			EventTracer->AuthorityChange(Op.op.component_set_authority_change, FSpatialGDKSpanId(Op.span_id));
			break;
		case WORKER_OP_TYPE_COMPONENT_UPDATE:
			EventTracer->UpdateComponent(Op.op.component_update, FSpatialGDKSpanId(Op.span_id));
			break;
		case WORKER_OP_TYPE_COMMAND_REQUEST:
			EventTracer->CommandRequest(Op.op.command_request, FSpatialGDKSpanId(Op.span_id));
			break;
		default:
			break;
		}
	}
}

} // namespace SpatialGDK
