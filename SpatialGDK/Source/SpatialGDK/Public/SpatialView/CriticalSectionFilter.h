// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "OpList/OpList.h"

namespace SpatialGDK
{
class FCriticalSectionFilter
{
public:
	void AddOpList(OpList Ops);
	TArray<OpList> GetReadyOpLists();

private:
	TArray<OpList> ReadyOps;
	TArray<OpList> OpenCriticalSectionOps;
};
} // namespace SpatialGDK
