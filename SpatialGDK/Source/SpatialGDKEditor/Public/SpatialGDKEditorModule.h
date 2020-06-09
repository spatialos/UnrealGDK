// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Improbable/SpatialGDKSettingsBridge.h"
#include "Modules/ModuleManager.h"

class FLBStrategyEditorExtensionManager;
class FSpatialGDKEditor;
class FSpatialGDKEditorCommandLineArgsManager;

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

	TSharedPtr<FSpatialGDKEditor> GetSpatialGDKEditorInstance() const
	{
		return SpatialGDKEditorInstance;
	}

protected:
	// Local deployment connection flow
	virtual bool ShouldConnectToLocalDeployment() const override;
	virtual FString GetSpatialOSLocalDeploymentIP() const override;
	virtual bool ShouldStartPIEClientsWithLocalLaunchOnDevice() const override;

	// Cloud deployment connection flow
	virtual bool ShouldConnectToCloudDeployment() const override;
	virtual FString GetDevAuthToken() const override;
	virtual FString GetSpatialOSCloudDeploymentName() const override;

	virtual bool CanExecuteLaunch() const override;

private:
	void RegisterSettings();
	void UnregisterSettings();
	bool HandleEditorSettingsSaved();
	bool HandleRuntimeSettingsSaved();
	bool HandleCloudLauncherSettingsSaved();

private:
	TUniquePtr<FLBStrategyEditorExtensionManager> ExtensionManager;
	TSharedPtr<FSpatialGDKEditor> SpatialGDKEditorInstance;
	TUniquePtr<FSpatialGDKEditorCommandLineArgsManager> CommandLineArgsManager;
};
