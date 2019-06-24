// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

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
};
