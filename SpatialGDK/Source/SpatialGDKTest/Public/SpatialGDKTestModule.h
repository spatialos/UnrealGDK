// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKTestModule, Log, All);

class FSpatialGDKTestModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};
