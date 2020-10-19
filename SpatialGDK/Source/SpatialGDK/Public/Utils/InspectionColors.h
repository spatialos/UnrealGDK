// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Math/Color.h"
#include "SpatialCommonTypes.h"

// Mimicking Inspector V2 coloring from platform/js/console/src/inspector-v2/styles/colors.ts

namespace SpatialGDK
{
// Argument expected in the form: UnrealWorker1a2s3d4f...
FColor GetColorForWorkerName(const PhysicalWorkerName& WorkerName);
} // namespace SpatialGDK
