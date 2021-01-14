// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Async/Future.h"
#include "CoreMinimal.h"
#include "Framework/SlateDelegates.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonWriter.h"
#include "Templates/SharedPointer.h"
#include "TickableEditorObject.h"
#include "UObject/UnrealType.h"
#include "Widgets/Notifications/SNotificationList.h"

#include "CloudDeploymentConfiguration.h"
#include "LocalDeploymentManager.h"
#include "LocalReceptionistProxyServerManager.h"
#include "SpatialGDKEditorSettings.h"

class FMenuBuilder;
class FSpatialGDKEditor;
class FToolBarBuilder;
class FUICommandList;
class SSpatialGDKCloudDeploymentConfiguration;
class SWindow;
class USoundBase;

struct FWorkerTypeLaunchSection;
class UAbstractRuntimeLoadBalancingStrategy;
class ASpatialDebugger;

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKEditorToolbar, Log, All);

class FSpatialGDKEditorToolbarModule : public IModuleInterface, public FTickableEditorObject
{
public:
	FSpatialGDKEditorToolbarModule();

	void StartupModule() override;
	void ShutdownModule() override;
	void PreUnloadCallback() override;

	/** FTickableEditorObject interface */
	void Tick(float DeltaTime) override;
	bool IsTickable() const override { return true; }

	TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(FSpatialGDKEditorToolbarModule, STATGROUP_Tickables); }

	void OnShowSingleFailureNotification(const FString& NotificationText);
	void OnShowSuccessNotification(const FString& NotificationText);
	void OnShowFailedNotification(const FString& NotificationText);
	void OnShowTaskStartNotification(const FString& NotificationText);

	FReply OnStartCloudDeployment();
	bool CanStartCloudDeployment() const;

	bool IsSimulatedPlayersEnabled() const;
	/** Delegate called when the user either clicks the simulated players checkbox */
	void OnCheckedSimulatedPlayers();

	bool IsBuildClientWorkerEnabled() const;
	void OnCheckedBuildClientWorker();

private:
	void MapActions(TSharedPtr<FUICommandList> PluginCommands);
	void SetupToolbar(TSharedPtr<FUICommandList> PluginCommands);
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	bool FetchRuntimeBinaryWrapper(FString RuntimeVersion);
	bool FetchInspectorBinaryWrapper(FString InspectorVersion);

	void VerifyAndStartDeployment(FString ForceSnapshot = "");

	void StartLocalSpatialDeploymentButtonClicked();
	void StopSpatialDeploymentButtonClicked();

	void MapChanged(UWorld* World, EMapChangeType MapChangeType);
	void DestroySpatialDebuggerEditor();
	void InitialiseSpatialDebuggerEditor(UWorld* World);
	bool IsSpatialDebuggerEditorEnabled() const;
	bool IsMultiWorkerEnabled() const;
	bool AllowWorkerBoundaries() const;
	void ToggleSpatialDebuggerEditor();

	void ToggleMultiworkerEditor();
	bool StartNativeIsVisible() const;
	bool StartNativeCanExecute() const;

	bool StartLocalSpatialDeploymentIsVisible() const;
	bool StartLocalSpatialDeploymentCanExecute() const;

	bool StartCloudSpatialDeploymentIsVisible() const;
	bool StartCloudSpatialDeploymentCanExecute() const;

	bool LaunchInspectorWebpageCanExecute() const;

	bool StopSpatialDeploymentIsVisible() const;
	bool StopSpatialDeploymentCanExecute() const;

	void OnToggleSpatialNetworking();
	bool OnIsSpatialNetworkingEnabled() const;

	void GDKEditorSettingsClicked() const;
	void GDKRuntimeSettingsClicked() const;

	bool IsLocalDeploymentSelected() const;
	bool IsCloudDeploymentSelected() const;

	bool IsSpatialOSNetFlowConfigurable() const;

	void LocalDeploymentClicked();
	void CloudDeploymentClicked();

	static bool IsLocalDeploymentIPEditable();
	static bool AreCloudDeploymentPropertiesEditable();

	void OpenInspectorURL();
	void LaunchInspectorWebpageButtonClicked();
	void CreateSnapshotButtonClicked();
	void SchemaGenerateButtonClicked();
	void SchemaGenerateFullButtonClicked();
	void DeleteSchemaDatabaseButtonClicked();
	void OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);

	void ShowCloudDeploymentDialog();
	void OpenLaunchConfigurationEditor();
	void LaunchOrShowCloudDeployment();

	/** Delegate to determine the 'Start Deployment' button enabled state */
	bool IsDeploymentConfigurationValid() const;
	bool CanBuildAndUpload() const;

	void OnBuildSuccess();
	void OnStartCloudDeploymentFinished();

	void AddDeploymentTagIfMissing(const FString& TagToAdd);

private:
	bool CanExecuteSchemaGenerator() const;
	bool CanExecuteSnapshotGenerator() const;

	TSharedRef<SWidget> CreateGenerateSchemaMenuContent();
	TSharedRef<SWidget> CreateLaunchDeploymentMenuContent();
	TSharedRef<SWidget> CreateStartDropDownMenuContent();

	using IsEnabledFunc = bool();
	TSharedRef<SWidget> CreateBetterEditableTextWidget(const FText& Label, const FText& Text, FOnTextCommitted::TFuncType OnTextCommitted,
													   IsEnabledFunc IsEnabled);

	void ShowSingleFailureNotification(const FString& NotificationText);
	void ShowTaskStartNotification(const FString& NotificationText);

	void ShowSuccessNotification(const FString& NotificationText);

	void ShowFailedNotification(const FString& NotificationText);

	void GenerateSchema(bool bFullScan);

	bool IsSnapshotGenerated() const;

	FString GetOptionalExposedRuntimeIP() const;

	// This should be called whenever the settings determining whether a local deployment should be automatically started have changed.
	void OnAutoStartLocalDeploymentChanged();

	TSharedPtr<FUICommandList> PluginCommands;
	FDelegateHandle OnPropertyChangedDelegateHandle;
	EAutoStopLocalDeploymentMode AutoStopLocalDeployment;

	bool bSchemaBuildError;

	TWeakPtr<SNotificationItem> TaskNotificationPtr;

	// Sounds used for execution of tasks.
	USoundBase* ExecutionStartSound;
	USoundBase* ExecutionSuccessSound;
	USoundBase* ExecutionFailSound;

	TFuture<bool> SchemaGeneratorResult;
	TSharedPtr<FSpatialGDKEditor> SpatialGDKEditorInstance;

	TSharedPtr<SWindow> CloudDeploymentSettingsWindowPtr;
	TSharedPtr<SSpatialGDKCloudDeploymentConfiguration> CloudDeploymentConfigPtr;

	FLocalDeploymentManager* LocalDeploymentManager;
	FLocalReceptionistProxyServerManager* LocalReceptionistProxyServerManager;

	TFuture<bool> AttemptSpatialAuthResult;

	FCloudDeploymentConfiguration CloudDeploymentConfiguration;

	bool bStartingCloudDeployment;
	bool bFetchingRuntimeBinary;
	bool bFetchingInspectorBinary;

	void GenerateCloudConfigFromCurrentMap();

	// Used to show worker boundaries in the editor
	TWeakObjectPtr<ASpatialDebugger> SpatialDebugger;

	TOptional<FMonitoredProcess> InspectorProcess = {};
};
