// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Improbable/SpatialGDKSettingsBridge.h"
#include "Modules/ModuleManager.h"

#include "Utils/EngineVersionCheck.h"

#include "SpatialGDKLoader.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKModule, Log, All);

class SPATIALGDK_API FSpatialGDKModule : public ISpatialGDKModule
{
public:
	void StartupModule() override;
	void ShutdownModule() override;

	virtual bool UsesSpatialNetworking() const override;
	virtual void SetUsesSpatialNetworking(bool bEnabled) override;

private:
	FSpatialGDKLoader Loader;
};
