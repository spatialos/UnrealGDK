// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "AbstractOpList.h"
#include "Containers/Array.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{

class ViewDeltaLegacyOpList : public AbstractOpList
{
public:
	explicit ViewDeltaLegacyOpList(TArray<Worker_Op> OpList)
	: OpList(MoveTemp(OpList))
	{
	}

	virtual uint32 GetCount() const override
	{
		return OpList.Num();
	}

	virtual Worker_Op& operator[](uint32 Index) override
	{
		return OpList[Index];
	}

	virtual const Worker_Op& operator[](uint32 Index) const override
	{
		return OpList[Index];
	}

private:
	TArray<Worker_Op> OpList;
};

}  // namespace SpatialGDK
