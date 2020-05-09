// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Improbable/SpatialGDKSettingsBridge.h"
#include "Modules/ModuleManager.h"

class FLBStrategyEditorExtensionManager;
class FSpatialGDKEditorCommandLineArgsManager;

class FSpatialGDKEditorModule : public ISpatialGDKEditorModule
{
public:

	FSpatialGDKEditorModule();

	SPATIALGDKEDITOR_API FLBStrategyEditorExtensionManager& GetLBStrategyExtensionManager() { return *ExtensionManager; }
	SPATIALGDKEDITOR_API FSpatialGDKEditorCommandLineArgsManager& GetCommandLineArgsManager() { return *CommandLineArgsManager; }

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

protected:
	// Local deployment connection flow
	virtual bool ShouldConnectToLocalDeployment() const override;
	virtual FString GetSpatialOSLocalDeploymentIP() const override;

	// Cloud deployment connection flow
	virtual bool ShouldConnectToCloudDeployment() const override;
	virtual FString GetDevAuthToken() const override;
	virtual FString GetSpatialOSCloudDeploymentName() const override;

private:
	void RegisterSettings();
	void UnregisterSettings();
	bool HandleEditorSettingsSaved();
	bool HandleRuntimeSettingsSaved();
	bool HandleCloudLauncherSettingsSaved();

	TUniquePtr<FLBStrategyEditorExtensionManager> ExtensionManager;

	TUniquePtr<FSpatialGDKEditorCommandLineArgsManager> CommandLineArgsManager;
};
