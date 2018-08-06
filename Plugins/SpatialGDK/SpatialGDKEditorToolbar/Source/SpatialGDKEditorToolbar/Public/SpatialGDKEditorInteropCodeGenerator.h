// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Runnable.h"
#include "RunnableThread.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKInteropCodeGenerator, Log, All);

using ClassHeaderMap = TMap<FString, TArray<FString>>;

bool GenerateClassHeaderMap(ClassHeaderMap& OutClasses);

bool SpatialGDKGenerateInteropCode(const ClassHeaderMap& InteropGeneratedClasses);
