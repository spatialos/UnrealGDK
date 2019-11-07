// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/OpUtils.h"
#include "SpatialConstants.h"

namespace SpatialGDK
{
void FindFirstOpOfType(const TArray<Worker_OpList*>& InOpLists, const Worker_OpType InOpType, Worker_Op** OutOp)
{
	for (const Worker_OpList* OpList : InOpLists)
	{
		for (size_t i = 0; i < OpList->op_count; ++i)
		{
			Worker_Op* Op = &OpList->ops[i];

			if (Op->op_type == InOpType)
			{
				*OutOp = Op;
				return;
			}
		}
	}
}

void FindFirstOpOfTypeForComponent(const TArray<Worker_OpList*>& InOpLists, const Worker_OpType InOpType, const Worker_ComponentId InComponentId, Worker_Op** OutOp)
{
	for (const Worker_OpList* OpList : InOpLists)
	{
		for (size_t i = 0; i < OpList->op_count; ++i)
		{
			Worker_Op* Op = &OpList->ops[i];

			if ((Op->op_type == InOpType) &&
				GetComponentId(Op) == InComponentId)
			{
				*OutOp = Op;
				return;
			}
		}
	}
}

Worker_ComponentId GetComponentId(const Worker_Op* Op)
{
	switch (Op->op_type)
	{
	case WORKER_OP_TYPE_ADD_COMPONENT:
		return Op->op.add_component.data.component_id;
	case WORKER_OP_TYPE_REMOVE_COMPONENT:
		return Op->op.remove_component.component_id;
	case WORKER_OP_TYPE_COMPONENT_UPDATE:
		return Op->op.component_update.update.component_id;
	case WORKER_OP_TYPE_AUTHORITY_CHANGE:
		return Op->op.authority_change.component_id;
	case WORKER_OP_TYPE_COMMAND_REQUEST:
		return Op->op.command_request.request.component_id;
	case WORKER_OP_TYPE_COMMAND_RESPONSE:
		return Op->op.command_response.response.component_id;
	default:
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}
} // namespace SpatialGDK
