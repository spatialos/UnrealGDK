// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/OpUtils.h"
#include "SpatialConstants.h"

namespace SpatialGDK
{
Worker_Op* FindFirstOpOfType(const TArray<OpList>& InOpLists, const Worker_OpType OpType)
{
	for (const OpList& Ops : InOpLists)
	{
		for (size_t i = 0; i < Ops.Count; ++i)
		{
			Worker_Op* Op = &Ops.Ops[i];

			if (Op->op_type == OpType)
			{
				return Op;
			}
		}
	}
	return nullptr;
}

void AppendAllOpsOfType(const TArray<OpList>& InOpLists, const Worker_OpType InOpType, TArray<Worker_Op*>& FoundOps)
{
	for (const OpList& Ops : InOpLists)
	{
		for (size_t i = 0; i < Ops.Count; ++i)
		{
			Worker_Op* Op = &Ops.Ops[i];

			if (Op->op_type == InOpType)
			{
				FoundOps.Add(Op);
			}
		}
	}
}

Worker_Op* FindFirstOpOfTypeForComponent(const TArray<SpatialGDK::OpList>& InOpLists, const Worker_OpType OpType,
										 const Worker_ComponentId ComponentId)
{
	for (const OpList& Ops : InOpLists)
	{
		for (size_t i = 0; i < Ops.Count; ++i)
		{
			Worker_Op* Op = &Ops.Ops[i];

			if ((Op->op_type == OpType) && GetComponentId(Op) == ComponentId)
			{
				return Op;
			}
		}
	}
	return nullptr;
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
