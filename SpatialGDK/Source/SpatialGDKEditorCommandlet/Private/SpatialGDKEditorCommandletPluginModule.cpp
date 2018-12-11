// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorCommandletPluginPrivate.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorCommandlet);

class FSpatialGDKEditorCommandletModule : public IModuleInterface
{
public:
	virtual void StartupModule() override { }
	virtual void ShutdownModule() override { }

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}
};

IMPLEMENT_MODULE(FSpatialGDKEditorCommandletModule, SpatialGDKEditorCommandlet);
