// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Async/Future.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonWriter.h"
#include "Templates/SharedPointer.h"
#include "TickableEditorObject.h"
#include "UObject/UnrealType.h"
#include "Widgets/Notifications/SNotificationList.h"

class FToolBarBuilder;
class FMenuBuilder;
class FUICommandList;
class USoundBase;
class FSpatialGDKEditor;

struct FWorkerTypeLaunchSection;

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
	bool IsTickable() const override
	{
		return true;
	}

	TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FSpatialGDKEditorToolbarModule, STATGROUP_Tickables);
	}

	FSimpleMulticastDelegate OnSpatialShutdown;

	FSimpleMulticastDelegate OnDeploymentStart;

	bool bLocalDeploymentRunning = false;
	bool bSpatialServiceRunning = false;

private:
	void MapActions(TSharedPtr<FUICommandList> PluginCommands);
	void SetupToolbar(TSharedPtr<FUICommandList> PluginCommands);
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	void StartSpatialDeploymentButtonClicked();
	void StopSpatialDeploymentButtonClicked();
	bool StartSpatialDeploymentIsVisible();
	bool StartSpatialDeploymentCanExecute();
	bool StartSpatialServiceIsVisible();
	bool StartSpatialServiceCanExecute();
	bool StopSpatialServiceIsVisible();
	bool StopSpatialServiceCanExecute();
	void RefreshServiceStatus();
	TSharedPtr<FJsonObject> ParseJson(FString RawJsonString);
	bool StopSpatialDeploymentIsVisible();

	bool StopSpatialDeploymentCanExecute();
	void LaunchInspectorWebpageButtonClicked();
	void CreateSnapshotButtonClicked();
	void SchemaGenerateButtonClicked();
	void SchemaGenerateFullButtonClicked();
	void OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);

private:
	bool CanExecuteSchemaGenerator() const;
	bool CanExecuteSnapshotGenerator() const;
	void StopRunningStack();
	void CheckForRunningStack();
	void CleanupSpatialProcess();

	TSharedRef<SWidget> CreateGenerateSchemaMenuContent();

	void ShowTaskStartNotification(const FString& NotificationText);
	void ShowSuccessNotification(const FString& NotificationText);
	void ShowFailedNotification(const FString& NotificationText);

	bool ValidateGeneratedLaunchConfig() const;
	bool IsSpatialServiceRunning();
	bool TryStartSpatialService();
	bool TryStopSpatialService();
	bool TryStopLocalDeployment();
	bool IsLocalDeploymentRunning();
	FString ExecuteAndReadOutput(FString Executable, FString Arguments, FString DirectoryToRun);
	void StartSpatialServiceButtonClicked();
	void StopSpatialServiceButtonClicked();
	void TryStartLocalDeployment();
	bool GenerateDefaultLaunchConfig(const FString& LaunchConfigPath) const;

	void GenerateSchema(bool bFullScan);

	bool WriteFlagSection(TSharedRef< TJsonWriter<> > Writer, const FString& Key, const FString& Value) const;
	bool WriteWorkerSection(TSharedRef< TJsonWriter<> > Writer, const FWorkerTypeLaunchSection& FWorkerTypeLaunchSection) const;
	bool WriteLoadbalancingSection(TSharedRef< TJsonWriter<> > Writer, const FString& WorkerType, const int32 Columns, const int32 Rows, const bool bManualWorkerConnectionOnly) const;

	static void ShowCompileLog();

	TSharedPtr<FUICommandList> PluginCommands;
	FDelegateHandle OnPropertyChangedDelegateHandle;
	FProcHandle SpatialOSStackProcHandle;
	bool bStopSpatialOnExit;
	
	uint32 SpatialOSStackProcessID;

	TWeakPtr<SNotificationItem> TaskNotificationPtr;

	// Sounds used for execution of tasks.
	USoundBase* ExecutionStartSound;
	USoundBase* ExecutionSuccessSound;
	USoundBase* ExecutionFailSound;

	TFuture<bool> SchemaGeneratorResult;
	TSharedPtr<FSpatialGDKEditor> SpatialGDKEditorInstance;

	FDateTime LastSpatialServiceCheck;
	FDateTime LastDeploymentCheck;

	bool bStartingDeployment = false;
	bool bStoppingDeployment = false;
	bool bStartingSpatialService = false;
	bool bStoppingSpatialService = false;

	FString LocalRunningDeploymentID;
};
