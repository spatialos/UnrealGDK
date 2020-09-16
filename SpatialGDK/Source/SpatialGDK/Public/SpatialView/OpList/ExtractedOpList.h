// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "OpList.h"

#include "Containers/Array.h"

namespace SpatialGDK
{
struct ExtractedOpListData : OpListData
{
	TArray<Worker_Op> ExtractedOps;

	void AddOp(Worker_Op& Op)
	{
		ExtractedOps.Push(Op);
		Op.op_type = 0;
	}
};

} // namespace SpatialGDK
