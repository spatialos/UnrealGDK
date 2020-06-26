// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Improbable/SpatialGDKSettingsBridge.h"
#include "Modules/ModuleManager.h"

class FLBStrategyEditorExtensionManager;
class FSpatialGDKEditor;
class FSpatialGDKEditorCommandLineArgsManager;
class  FLocalReceptionistProxyServerManager;

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

private:
	// Local deployment connection flow
	virtual bool ShouldConnectToLocalDeployment() const override;
	virtual FString GetSpatialOSLocalDeploymentIP() const override;
	virtual bool ShouldStartPIEClientsWithLocalLaunchOnDevice() const override;

	// Cloud deployment connection flow
	virtual bool ShouldConnectToCloudDeployment() const override;
	virtual FString GetDevAuthToken() const override;
	virtual FString GetSpatialOSCloudDeploymentName() const override;
	virtual bool ShouldConnectServerToCloud() const override;
	virtual bool TryStartLocalReceptionistProxyServer() const override;

	virtual bool CanExecuteLaunch() const override;
	virtual bool CanStartPlaySession(FText& OutErrorMessage) const override;
	virtual bool CanStartLaunchSession(FText& OutErrorMessage) const override;

	virtual FString GetMobileClientCommandLineArgs() const override;
	virtual bool ShouldPackageMobileCommandLineArgs() const override;

	virtual bool ShouldStartLocalServer() const override;
private:
	void RegisterSettings();
	void UnregisterSettings();
	bool HandleEditorSettingsSaved();
	bool HandleRuntimeSettingsSaved();
	bool HandleCloudLauncherSettingsSaved();
	bool CanStartSession(FText& OutErrorMessage) const;

private:
	TUniquePtr<FLBStrategyEditorExtensionManager> ExtensionManager;
	TSharedPtr<FSpatialGDKEditor> SpatialGDKEditorInstance;
	TUniquePtr<FSpatialGDKEditorCommandLineArgsManager> CommandLineArgsManager;

	FLocalReceptionistProxyServerManager* LocalReceptionistProxyServerManager;
};
