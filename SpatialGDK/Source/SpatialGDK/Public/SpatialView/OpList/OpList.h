// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Templates/UniquePtr.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{
struct OpListData
{
	virtual ~OpListData() = default;
};

struct OpList
{
	Worker_Op* Ops;
	uint32 Count;
	TUniquePtr<OpListData> Storage;
};

} // namespace SpatialGDK
