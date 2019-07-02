// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Async/Future.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialDeploymentManager, Log, All);

struct FFileChangeData;
struct FJsonObject;

class FLocalDeploymentManager
{
public:
	FLocalDeploymentManager();

	FSimpleMulticastDelegate OnSpatialShutdown;
	FSimpleMulticastDelegate OnDeploymentStart;

	FDelegateHandle WorkerConfigDirectoryChangedDelegateHandle;
	IDirectoryWatcher::FDirectoryChanged WorkerConfigDirectoryChangedDelegate;

	void SPATIALGDKSERVICES_API RefreshServiceStatus();

	bool SPATIALGDKSERVICES_API IsLocalDeploymentRunning();
	bool SPATIALGDKSERVICES_API IsSpatialServiceRunning();

	bool SPATIALGDKSERVICES_API TryStartLocalDeployment(FString LaunchConfig, FString LaunchArgs);
	bool SPATIALGDKSERVICES_API TryStopLocalDeployment();

	bool SPATIALGDKSERVICES_API TryStartSpatialService();
	bool SPATIALGDKSERVICES_API TryStopSpatialService();

	bool SPATIALGDKSERVICES_API GetLocalDeploymentStatus();
	bool SPATIALGDKSERVICES_API GetServiceStatus();

	bool SPATIALGDKSERVICES_API IsDeploymentStarting();
	bool SPATIALGDKSERVICES_API IsDeploymentStopping();

	bool SPATIALGDKSERVICES_API IsServiceStarting();
	bool SPATIALGDKSERVICES_API IsServiceStopping();

	// TODO: Refactor these into Utils
	FString GetProjectName();
	void WorkerBuildConfigAsync();
	bool ParseJson(FString RawJsonString, TSharedPtr<FJsonObject>& JsonParsed);
	void ExecuteAndReadOutput(FString Executable, FString Arguments, FString DirectoryToRun, FString& OutResult, int32& ExitCode);
	FString GetSpotExe();
	FString GetSpatialOSDirectory();

private:
	bool bLocalDeploymentRunning;
	bool bSpatialServiceRunning;

	bool bStartingDeployment;
	bool bStoppingDeployment;

	bool bStartingSpatialService;
	bool bStoppingSpatialService;

	FDateTime LastSpatialServiceCheck;
	FDateTime LastDeploymentCheck;

	FString LocalRunningDeploymentID;
	FString ProjectName;

	void StartUpWorkerConfigDirectoryWatcher();
	void OnWorkerConfigDirectoryChanged(const TArray<FFileChangeData>& FileChanges);

	static const int32 ExitCodeSuccess = 0;
	static const int32 RefreshFrequency = 3;
};
