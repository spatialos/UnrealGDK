// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Async/Future.h"
#include "CoreMinimal.h"
#include "LocalDeploymentManager.h"
#include "Modules/ModuleManager.h"
#include "Serialization/JsonWriter.h"
#include "Templates/SharedPointer.h"
#include "TickableEditorObject.h"
#include "UObject/UnrealType.h"
#include "Widgets/Notifications/SNotificationList.h"

class FMenuBuilder;
class FSpatialGDKEditor;
class FToolBarBuilder;
class FUICommandList;
class SSpatialGDKSimulatedPlayerDeployment;
class SWindow;
class USoundBase;

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

private:
	void MapActions(TSharedPtr<FUICommandList> PluginCommands);
	void SetupToolbar(TSharedPtr<FUICommandList> PluginCommands);
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	void VerifyAndStartDeployment();

	void StartSpatialDeploymentButtonClicked();
	void StopSpatialDeploymentButtonClicked();

	void StartSpatialServiceButtonClicked();
	void StopSpatialServiceButtonClicked();

	bool StartSpatialDeploymentIsVisible() const;
	bool StartSpatialDeploymentCanExecute() const;

	bool StopSpatialDeploymentIsVisible() const;
	bool StopSpatialDeploymentCanExecute() const;

	bool StartSpatialServiceIsVisible() const;
	bool StartSpatialServiceCanExecute() const;

	bool StopSpatialServiceIsVisible() const;
	bool StopSpatialServiceCanExecute() const;

	void LaunchInspectorWebpageButtonClicked();
	void CreateSnapshotButtonClicked();
	void SchemaGenerateButtonClicked();
	void SchemaGenerateFullButtonClicked();
	void OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);

	void ShowSimulatedPlayerDeploymentDialog();

private:
	bool CanExecuteSchemaGenerator() const;
	bool CanExecuteSnapshotGenerator() const;

	TSharedRef<SWidget> CreateGenerateSchemaMenuContent();

	void OnShowTaskStartNotification(const FString& NotificationText);
	void ShowTaskStartNotification(const FString& NotificationText);

	void OnShowSuccessNotification(const FString& NotificationText);
	void ShowSuccessNotification(const FString& NotificationText);

	void OnShowFailedNotification(const FString& NotificationText);
	void ShowFailedNotification(const FString& NotificationText);

	bool ValidateGeneratedLaunchConfig() const;
	bool GenerateDefaultLaunchConfig(const FString& LaunchConfigPath) const;
	bool GenerateDefaultWorkerJson();

	void GenerateSchema(bool bFullScan);

	bool WriteFlagSection(TSharedRef< TJsonWriter<> > Writer, const FString& Key, const FString& Value) const;
	bool WriteWorkerSection(TSharedRef< TJsonWriter<> > Writer, const FWorkerTypeLaunchSection& FWorkerTypeLaunchSection) const;
	bool WriteLoadbalancingSection(TSharedRef< TJsonWriter<> > Writer, const FName& WorkerType, const int32 Columns, const int32 Rows, const bool bManualWorkerConnectionOnly) const;

	static void ShowCompileLog();

	TSharedPtr<FUICommandList> PluginCommands;
	FDelegateHandle OnPropertyChangedDelegateHandle;
	bool bStopSpatialOnExit;

	TWeakPtr<SNotificationItem> TaskNotificationPtr;

	// Sounds used for execution of tasks.
	USoundBase* ExecutionStartSound;
	USoundBase* ExecutionSuccessSound;
	USoundBase* ExecutionFailSound;

	TFuture<bool> SchemaGeneratorResult;
	TSharedPtr<FSpatialGDKEditor> SpatialGDKEditorInstance;

	TSharedPtr<SWindow> SimulatedPlayerDeploymentWindowPtr;
	TSharedPtr<SSpatialGDKSimulatedPlayerDeployment> SimulatedPlayerDeploymentConfigPtr;
	
	FLocalDeploymentManager* LocalDeploymentManager;
};
