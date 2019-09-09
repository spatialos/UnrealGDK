// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include <WorkerSDK/improbable/c_worker.h>

class ASpatialWorldSettings;

namespace SpatialGDK
{
bool ShouldClassPersist(const ASpatialWorldSettings* SpatialWorldSettings, const UClass* Class);
} // namespace SpatialGDK
