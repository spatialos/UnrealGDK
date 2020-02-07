// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialCommandUtils, Log, All);

class SpatialCommandUtils
{
public:

	SPATIALGDKSERVICES_API static bool AttemptSpatialAuth(bool IsRunningInChina);
};