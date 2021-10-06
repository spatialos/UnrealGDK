// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "SpatialView/OpList/OpList.h"
#include "SpatialView/ViewDelta.h"

namespace SpatialGDK
{
/** Creates an OpList from a ViewDelta. */
TArray<Worker_Op> GetOpsFromEntityDeltas(const TArray<EntityDelta>& Deltas);

} // namespace SpatialGDK
