// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once
#pragma optimize("", on)

#include "Improbable/SpatialGDKSettingsBridge.h"
#include "Modules/ModuleManager.h"
#include "SpatialGDKLogParser.h"

class FSpatialGDKEditor;
class FSpatialGDKEditorCommandLineArgsManager;
class FLocalReceptionistProxyServerManager;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorModule, Log, All);

class FSpatialGDKEditorModule : public ISpatialGDKEditorModule
{
public:
	FSpatialGDKEditorModule();

	virtual void StartupModule() override;
	virtual void RevertSettingsOverrideForTesting() const;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override { return true; }

	TSharedPtr<FSpatialGDKEditor> GetSpatialGDKEditorInstance() const { return SpatialGDKEditorInstance; }

	virtual void TakeSnapshot(UWorld* World, FSpatialSnapshotTakenFunc OnSnapshotTaken) override;

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

	virtual FPlayInEditorSettingsOverride GetPlayInEditorSettingsOverrideForTesting(UWorld* World, const FString& MapName) const;

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

	const FString TmpSpatialGDKSettingsFilename = FPaths::GeneratedConfigDir().Append(TEXT("\\TmpSpatialGDKSettings.ini"));
	const FString TmpSpatialGDKEditorSettingsFilename = FPaths::GeneratedConfigDir().Append(TEXT("\\TmpSpatialGDKEditorSettings.ini"));
	const FString TmpLevelEditorPlaySettingsFilename = FPaths::GeneratedConfigDir().Append(TEXT("\\TmpLevelEditorPlaySettings.ini"));
	const FString TmpGeneralProjectSettingsFilename = FPaths::GeneratedConfigDir().Append(TEXT("\\TmpGeneralProjectSettings.ini"));
	const FString OverrideSettingsBaseFilename = FPaths::ProjectConfigDir().Append(TEXT("TestOverrides"));
	const FString OverrideSettingsFileExtension = ".ini";
};
