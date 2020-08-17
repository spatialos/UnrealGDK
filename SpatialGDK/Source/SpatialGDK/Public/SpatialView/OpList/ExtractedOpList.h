// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "OpList.h"

#include "Containers/Array.h"

namespace SpatialGDK
{
struct ExtractedOpListData : OpListData
{
	TArray<Worker_Op> ExtractedOps;

	void AddOp(OpList& Ops, uint32 Index)
	{
		Worker_Op& Op = Ops.Ops[Index];
		ExtractedOps.Push(Op);
		Op.op_type = 0;
	}
};

} // namespace SpatialGDK
