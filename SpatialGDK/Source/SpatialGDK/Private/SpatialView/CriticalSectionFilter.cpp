// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialView/CriticalSectionFilter.h"

#include "SpatialView/OpList/SplitOpList.h"

namespace SpatialGDK
{
void FCriticalSectionFilter::AddOpList(OpList Ops)
{
	// Work out which ops are in an open critical section by scanning backwards to the first critical sections op.
	// Critical sections can't overlap so if the first op we find (the last one in the list) tells us the status of all received ops,
	//
	// If the first critical section op found is closed, we know that there are no open critical sections.
	// In this case we expose all received ops as ready.
	//
	// If the first critical section op we find is open, we know that all subsequent ops are in an open critical section.
	// In this case we split the op list around the open critical section start, set the first part to ready and queue the second.
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
