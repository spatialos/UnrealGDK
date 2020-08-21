// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Async/Future.h"
#include "CoreMinimal.h"
#include "FileCache.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "TimerManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialDeploymentManager, Log, All);

class FJsonObject;

class FLocalDeploymentManager
{
public:
	FLocalDeploymentManager();

	void SPATIALGDKSERVICES_API PreInit(bool bChinaEnabled);

	void SPATIALGDKSERVICES_API Init(FString RuntimeIPToExpose);

	void SPATIALGDKSERVICES_API RefreshServiceStatus();

	bool CheckIfPortIsBound(int32 Port);
	bool KillProcessBlockingPort(int32 Port);
	bool LocalDeploymentPreRunChecks();

	using LocalDeploymentCallback = TFunction<void(bool)>;

	void SPATIALGDKSERVICES_API TryStartLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs,
														FString SnapshotName, FString RuntimeIPToExpose,
														const LocalDeploymentCallback& CallBack);
	bool SPATIALGDKSERVICES_API TryStopLocalDeployment();

	bool SPATIALGDKSERVICES_API TryStartSpatialService(FString RuntimeIPToExpose);
	bool SPATIALGDKSERVICES_API TryStopSpatialService();

	bool SPATIALGDKSERVICES_API GetLocalDeploymentStatus();
	bool SPATIALGDKSERVICES_API IsServiceRunningAndInCorrectDirectory();

	bool SPATIALGDKSERVICES_API IsLocalDeploymentRunning() const;
	bool SPATIALGDKSERVICES_API IsSpatialServiceRunning() const;

	bool SPATIALGDKSERVICES_API IsDeploymentStarting() const;
	bool SPATIALGDKSERVICES_API IsDeploymentStopping() const;

	bool SPATIALGDKSERVICES_API IsServiceStarting() const;
	bool SPATIALGDKSERVICES_API IsServiceStopping() const;

	bool SPATIALGDKSERVICES_API IsRedeployRequired() const;
	void SPATIALGDKSERVICES_API SetRedeployRequired();

	// Helper function to inform a client or server whether it should wait for a local deployment to become active.
	bool SPATIALGDKSERVICES_API ShouldWaitForDeployment() const;

	void SPATIALGDKSERVICES_API SetAutoDeploy(bool bAutoDeploy);

	void WorkerBuildConfigAsync();

	FSimpleMulticastDelegate OnSpatialShutdown;
	FSimpleMulticastDelegate OnDeploymentStart;

	FDelegateHandle WorkerConfigDirectoryChangedDelegateHandle;
	IDirectoryWatcher::FDirectoryChanged WorkerConfigDirectoryChangedDelegate;

private:
	void StartUpWorkerConfigDirectoryWatcher();
	void OnWorkerConfigDirectoryChanged(const TArray<FFileChangeData>& FileChanges);

	bool FinishLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs, FString SnapshotName,
							   FString RuntimeIPToExpose);

	TFuture<bool> AttemptSpatialAuthResult;

	static const int32 ExitCodeSuccess = 0;
	static const int32 ExitCodeNotRunning = 4;
	static const int32 RequiredRuntimePort = 5301;

	// This is the frequency at which check the 'spatial service status' to ensure we have the correct state as the user can change spatial
	// service outside of the editor.
	static const int32 RefreshFrequency = 3;

	bool bLocalDeploymentManagerEnabled = true;

	bool bLocalDeploymentRunning;
	bool bSpatialServiceRunning;
	bool bSpatialServiceInProjectDirectory;

	bool bStartingDeployment;
	bool bStoppingDeployment;

	bool bStartingSpatialService;
	bool bStoppingSpatialService;

	FString ExposedRuntimeIP;

	FString LocalRunningDeploymentID;

	bool bRedeployRequired = false;
	bool bAutoDeploy = false;
	bool bIsInChina = false;
};
