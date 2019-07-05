// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "LocalDeploymentManager.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class SPATIALGDKSERVICES_API FSpatialGDKServicesModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

	FLocalDeploymentManager* GetLocalDeploymentManager();

	static FString GetSpatialOSDirectory(FString RelativePath = TEXT(""));
	static FString GetSpatialGDKPluginDirectory(FString RelativePath = TEXT(""));

private:
	FLocalDeploymentManager LocalDeploymentManager;
};
