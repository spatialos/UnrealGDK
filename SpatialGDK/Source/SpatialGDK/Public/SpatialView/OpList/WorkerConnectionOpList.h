// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/OpList/OpList.h"
#include "Templates/UniquePtr.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{
struct WorkerConnectionOpListData : OpListData
{
	struct Deleter
	{
		void operator()(Worker_OpList* OpListToDelete) const noexcept
		{
			if (OpListToDelete != nullptr)
			{
				Worker_OpList_Destroy(OpListToDelete);
			}
		}
	};

	TUniquePtr<Worker_OpList, Deleter> OpList;

	explicit WorkerConnectionOpListData(Worker_OpList* OpList)
		: OpList(OpList)
	{
	}
};

inline OpList GetOpListFromConnection(Worker_Connection* Connection)
{
	Worker_OpList* Ops = Worker_Connection_GetOpList(Connection, 0);

	for (uint32 i = 0; i < Ops->op_count; ++i)
	{
		Worker_Op& Op = Ops->ops[i];
		if (Op.op_type == WORKER_OP_TYPE_REMOVE_ENTITY)
		{
			UE_LOG(LogTemp, Log, TEXT("GetOpListFromConnection REMOVE_ENTITY Ops[%d] %d %lld"), i, Op.op_type,
				   Op.op.remove_entity.entity_id);
		}
		if (Op.op_type == WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE)
		{
			UE_LOG(LogTemp, Log, TEXT("GetOpListFromConnection DELETE_ENTITY_RESPONSE Ops[%d] %d %lld"), i, Op.op_type,
				   Op.op.delete_entity_response.entity_id);
		}
		if (Op.op_type == WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE)
		{
			UE_LOG(LogTemp, Log, TEXT("GetOpListFromConnection WORKER_OP_TYPE_COMPONENT_SET_AUTHORITY_CHANGE Ops[%d] %d %lld, %d"), i,
				   Op.op_type, Op.op.component_set_authority_change.entity_id, Op.op.component_set_authority_change.authority);
		}
	}
	return { Ops->ops, Ops->op_count, MakeUnique<WorkerConnectionOpListData>(Ops) };
}

} // namespace SpatialGDK
