// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/OpList/OpList.h"
#include "Templates/UniquePtr.h"
#include <improbable/c_worker.h>
#include <memory>

namespace SpatialGDK
{

struct WorkerConnectionOpListData : OpListData
{
	struct Deleter
	{
		void operator()(Worker_OpList* OpList) const noexcept
		{
			if (OpList != nullptr)
			{
				Worker_OpList_Destroy(OpList);
			}
		}
	};

	TUniquePtr<Worker_OpList, Deleter> OpList;

	explicit WorkerConnectionOpListData(Worker_OpList* OpList) : OpList(OpList)
	{
	}
};

inline OpList GetOpListFromConnection(Worker_Connection* Connection)
{
	Worker_OpList* Ops = Worker_Connection_GetOpList(Connection, 0);
	return {Ops->ops, Ops->op_count, MakeUnique<WorkerConnectionOpListData>(Ops)};
}

}  // namespace SpatialGDK
