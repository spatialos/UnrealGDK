// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "Utils/EngineVersionCheck.h"
#include "Utils/WorkerVersionCheck.h"

#include "SpatialGDKLoader.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKModule, Log, All);

class SPATIALGDK_API FSpatialGDKModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

	void PrintCustomAllocateInfo();
	size_t DeltaSize;
	size_t LastSize;

private:
	FSpatialGDKLoader Loader;
};
