// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/CriticalSectionFilter.h"

#include "SpatialView/OpList/SplitOpList.h"

namespace SpatialGDK
{
void FCriticalSectionFilter::AddOpList(OpList Ops)
{ // Ensure that we only process closed critical sections.
	// Scan backwards looking for critical sections ops.
	for (uint32 i = Ops.Count; i > 0; --i)
	{
		Worker_Op& Op = Ops.Ops[i - 1];
		if (Op.op_type != WORKER_OP_TYPE_CRITICAL_SECTION)
		{
			continue;
		}

		// There can only be one critical section open at a time.
		// So any previous open critical section must now be closed.
		for (OpList& OpenCriticalSection : OpenCriticalSectionOps)
		{
			ReadyOps.Add(MoveTemp(OpenCriticalSection));
		}
		OpenCriticalSectionOps.Empty();

		// If critical section op is opening the section then enqueue any ops before this point and store the open critical section.
		if (Op.op.critical_section.in_critical_section)
		{
			SplitOpListPair SplitOpLists(MoveTemp(Ops), i);
			ReadyOps.Add(MoveTemp(SplitOpLists.Head));
			OpenCriticalSectionOps.Add(MoveTemp(SplitOpLists.Tail));
		}
		// If critical section op is closing the section then enqueue all ops.
		else
		{
			ReadyOps.Add(MoveTemp(Ops));
		}
		return;
	}

	// If no critical section is present then either add this to existing open section ops if there are any or enqueue if not.
	if (OpenCriticalSectionOps.Num())
	{
		OpenCriticalSectionOps.Push(MoveTemp(Ops));
	}
	else
	{
		ReadyOps.Push(MoveTemp(Ops));
	}
}

TArray<OpList> FCriticalSectionFilter::GetReadyOpLists()
{
	TArray<OpList> Temp = MoveTemp(ReadyOps);
	ReadyOps.Empty();
	return Temp;
}
} // namespace SpatialGDK
