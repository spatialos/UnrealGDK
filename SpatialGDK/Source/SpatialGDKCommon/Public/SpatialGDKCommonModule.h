// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class SPATIALGDKCOMMON_API FSpatialGDKCommonModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}
};
