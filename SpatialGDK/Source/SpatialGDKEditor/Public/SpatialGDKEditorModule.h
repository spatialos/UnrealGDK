// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Improbable/SpatialGDKSettingsBridge.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FLBStrategyEditorExtensionManager;

class FSpatialGDKEditorModule : public ISpatialGDKEditorModule
{
public:

	FSpatialGDKEditorModule();

	SPATIALGDKEDITOR_API FLBStrategyEditorExtensionManager& GetLBStrategyExtensionManager() { return *ExtensionManager; }

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

protected:
	virtual FString GetSpatialOSCloudDeploymentName() const override;
	virtual FString GetSpatialOSLocalDeploymentIP() const override;
	virtual bool ShouldConnectToLocalDeployment() const override;
	virtual bool ShouldConnectToCloudDeployment() const override;
	virtual FString GetDevAuthToken() const override;

private:
	void RegisterSettings();
	void UnregisterSettings();
	bool HandleEditorSettingsSaved();
	bool HandleRuntimeSettingsSaved();
	bool HandleCloudLauncherSettingsSaved();

	TUniquePtr<FLBStrategyEditorExtensionManager> ExtensionManager;
};
