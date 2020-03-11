// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FLBStrategyEditorExtensionManager;

class FSpatialGDKEditorModule : public IModuleInterface
{
public:

	FSpatialGDKEditorModule();

	SPATIALGDKEDITOR_API const FLBStrategyEditorExtensionManager& GetLBStrategyExtensionManager() { return *ExtensionManager; }

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

private:
	void RegisterSettings();
	void UnregisterSettings();
	bool HandleEditorSettingsSaved();
	bool HandleRuntimeSettingsSaved();
	bool HandleCloudLauncherSettingsSaved();

	TUniquePtr<FLBStrategyEditorExtensionManager> ExtensionManager;
};
