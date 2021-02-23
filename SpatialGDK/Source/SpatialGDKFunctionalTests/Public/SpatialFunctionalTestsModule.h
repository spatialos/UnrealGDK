// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Modules/ModuleInterface.h"

class FSpatialFunctionalTestsModule : public IModuleInterface
{
public:
	FSpatialFunctionalTestsModule();

	virtual void StartupModule() override;

	static void OverrideSettingsForTesting(UWorld* World, const FString& MapName);
};
