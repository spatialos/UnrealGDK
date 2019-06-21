// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKServicesPrivate.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FSpatialGDKServicesModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}
};
