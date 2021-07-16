// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/LBDataStorage.h"

namespace SpatialGDK
{
class FSubView;

class FPartitionSystemImpl
{
public:
	FPartitionSystemImpl(const FSubView& SubView)
		: PartitionData(SubView)
	{
	}

	FLBDataCollection PartitionData;
};
} // namespace SpatialGDK
