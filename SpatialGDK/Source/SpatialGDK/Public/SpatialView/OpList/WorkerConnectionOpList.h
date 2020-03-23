// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "AbstractOpList.h"
#include "UniquePtr.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{

class WorkerConnectionOpList : public AbstractOpList
{
public:
	explicit WorkerConnectionOpList(Worker_OpList* OpList)
	: OpList(OpList)
	{
	}

	virtual uint32 GetCount() const override
	{
		return OpList->op_count;
	}

	virtual Worker_Op& operator[](uint32 Index) override
	{
		return OpList->ops[Index];
	}

	virtual const Worker_Op& operator[](uint32 Index) const override
	{
		return OpList->ops[Index];
	}

private:
	struct Deleter
	{
		void operator()(Worker_OpList* Ops) const noexcept
		{
			Worker_OpList_Destroy(Ops);
		}
	};

	TUniquePtr<Worker_OpList, Deleter> OpList;
};

}  // namespace SpatialGDK
