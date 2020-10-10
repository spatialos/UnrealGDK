// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/CommonTypes.h"
#include "SpatialView/OpList/OpList.h"

#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
FComponentId GetComponentId(const Worker_Op& Op);

} // namespace SpatialGDK
