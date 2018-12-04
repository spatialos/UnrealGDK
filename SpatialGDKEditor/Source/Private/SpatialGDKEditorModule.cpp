// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorPrivate.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditor);

class FSpatialGDKEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override { }
	virtual void ShutdownModule() override { }

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}
};

IMPLEMENT_MODULE(FSpatialGDKEditorModule, SpatialGDKEditor);
