// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Improbable/SpatialGDKSettingsBridge.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FLBStrategyEditorExtensionManager;

class FSpatialGDKEditorModule : public ISpatialGDKEditorModule
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

protected:
	virtual FString GetSpatialOSLocalDeploymentIP() const override;
	virtual int GetSpatialOSNetFlowType() const override;

private:
	void RegisterSettings();
	void UnregisterSettings();
	bool HandleEditorSettingsSaved();
	bool HandleRuntimeSettingsSaved();
	bool HandleCloudLauncherSettingsSaved();

	TUniquePtr<FLBStrategyEditorExtensionManager> ExtensionManager;
};
