// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Improbable/SpatialGDKSettingsBridge.h"
#include "SpatialGDKLogParser.h"
#include "SpatialTestSettings.h"

class FSpatialGDKEditor;
class FSpatialGDKEditorCommandLineArgsManager;
class FLocalReceptionistProxyServerManager;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorModule, Log, All);

class FSpatialGDKEditorModule : public ISpatialGDKEditorModule
{
public:
	FSpatialGDKEditorModule();

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override { return true; }

	TSharedPtr<FSpatialGDKEditor> GetSpatialGDKEditorInstance() const { return SpatialGDKEditorInstance; }

	virtual void TakeSnapshot(UWorld* World, FSpatialSnapshotTakenFunc OnSnapshotTaken) override;

	// Delegate which others (specifically the SpatialFunctionalTestsModule) can bind to, to execute their functionality for overriding
	// settings and other things
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOverrideSettingsForTestingDelegate, UWorld*, const FString&);
	FOverrideSettingsForTestingDelegate OverrideSettingsForTestingDelegate;
	/* Way to force a deployment to be launched with a specific snapshot. This is meant to be override-able only
	 * at runtime, specifically for Functional Testing purposes.
	 */
	FString ForceUseSnapshotAtPath;

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

	virtual bool ForEveryServerWorker(TFunction<void(const FName&, int32)> Function) const override;

	virtual void OverrideSettingsForTesting(UWorld* World, const FString& MapName) override;

	virtual void RevertSettingsForTesting();

	virtual bool UsesActorInteractionSemantics() const override;

private:
	void RegisterSettings();
	void UnregisterSettings();
	bool HandleEditorSettingsSaved();
	bool HandleRuntimeSettingsSaved();
	bool CanStartSession(FText& OutErrorMessage) const;
	bool ShouldStartLocalServer() const;

private:
	FSpatialGDKLogParser LogParser;

	TSharedPtr<FSpatialGDKEditor> SpatialGDKEditorInstance;
	TUniquePtr<FSpatialGDKEditorCommandLineArgsManager> CommandLineArgsManager;

	FLocalReceptionistProxyServerManager* LocalReceptionistProxyServerManager;

	TUniquePtr<FSpatialTestSettings> SpatialTestSettings;
};
