// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#include "OpList.h"
#include "Templates/SharedPointer.h"

namespace SpatialGDK
{
struct SplitOpListData : OpListData
{
	TSharedPtr<OpListData> Data;

	explicit SplitOpListData(TSharedPtr<OpListData> SplitData)
		: Data(MoveTemp(SplitData))
	{
	}
};

struct SplitOpListPair
{
	SplitOpListPair(OpList OriginalOpList, uint32 InitialOpListCount)
	{
		check(InitialOpListCount <= OriginalOpList.Count);
		// Transfer ownership to a shared pointer.
		TSharedPtr<OpListData> SplitData(OriginalOpList.Storage.Release());
		Head = { OriginalOpList.Ops, InitialOpListCount, MakeUnique<SplitOpListData>(SplitData) };
		Tail = { OriginalOpList.Ops + InitialOpListCount, OriginalOpList.Count - InitialOpListCount,
				 MakeUnique<SplitOpListData>(MoveTemp(SplitData)) };
	}

	OpList Head;
	OpList Tail;
};

} // namespace SpatialGDK
