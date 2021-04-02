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

	static inline FSpatialGDKModule& Get()
	{
		static const FName ModuleName = "SpatialGDKModule";
		return FModuleManager::LoadModuleChecked<FSpatialGDKModule>(ModuleName);
	}
	
	void StartupModule() override;
	void ShutdownModule() override;

	void ShowAllocateStatus();

private:
	FSpatialGDKLoader Loader;
};
