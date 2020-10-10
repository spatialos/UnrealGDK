// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/OpList/OpList.h"

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
FComponentId GetComponentId(const Worker_Op& Op);

} // namespace SpatialGDK
