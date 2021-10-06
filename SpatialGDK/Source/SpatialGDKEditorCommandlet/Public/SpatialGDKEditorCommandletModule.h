// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialGDKEditorCommandletPrivate.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FSpatialGDKEditorCommandletModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override { return true; }
};
