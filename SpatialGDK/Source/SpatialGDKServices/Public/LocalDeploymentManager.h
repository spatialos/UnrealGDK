// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Async/Future.h"
#include "CoreMinimal.h"
#include "FileCache.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "TimerManager.h"
#include "LocalDeploymentManagerInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialDeploymentManager, Log, All);

class FJsonObject;

class FLocalDeploymentManager : public ILocalDeploymentManagerInterface
{
public:
	FLocalDeploymentManager();

	void PreInit(bool bChinaEnabled) override;

	void Init(FString RuntimeIPToExpose) override;

	void RefreshServiceStatus();

	bool CheckIfPortIsBound(int32 Port) override;
	bool KillProcessBlockingPort(int32 Port) override;
	bool LocalDeploymentPreRunChecks() override;

	void TryStartLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs, FString SnapshotName, FString RuntimeIPToExpose, const LocalDeploymentCallback& CallBack) override;
	bool TryStopLocalDeployment() override;

	bool SPATIALGDKSERVICES_API TryStartSpatialService(FString RuntimeIPToExpose);
	bool SPATIALGDKSERVICES_API TryStopSpatialService();

	bool GetLocalDeploymentStatus() override;
	bool SPATIALGDKSERVICES_API IsServiceRunningAndInCorrectDirectory();

	bool CanStartLocalDeployment() const override;
	bool CanStopLocalDeployment() const override;

	bool IsLocalDeploymentRunning() const override;
	bool SPATIALGDKSERVICES_API IsSpatialServiceRunning() const;

	virtual bool IsDeploymentStarting() const override;
	virtual bool IsDeploymentStopping() const override;

	bool SPATIALGDKSERVICES_API IsServiceStarting() const;
	bool SPATIALGDKSERVICES_API IsServiceStopping() const;

	bool IsRedeployRequired() const override;
	void SetRedeployRequired() override;

	// Helper function to inform a client or server whether it should wait for a local deployment to become active.
	bool ShouldWaitForDeployment() const override;

	void SetAutoDeploy(bool bAutoDeploy) override;

	void WorkerBuildConfigAsync();

	FDelegateHandle WorkerConfigDirectoryChangedDelegateHandle;
	IDirectoryWatcher::FDirectoryChanged WorkerConfigDirectoryChangedDelegate;

private:
	void StartUpWorkerConfigDirectoryWatcher();
	void OnWorkerConfigDirectoryChanged(const TArray<FFileChangeData>& FileChanges);

	bool FinishLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs, FString SnapshotName, FString RuntimeIPToExpose) override;

	TFuture<bool> AttemptSpatialAuthResult;

	static const int32 ExitCodeSuccess = 0;
	static const int32 ExitCodeNotRunning = 4;
	static const int32 RequiredRuntimePort = 5301;

	// This is the frequency at which check the 'spatial service status' to ensure we have the correct state as the user can change spatial service outside of the editor.
	static const int32 RefreshFrequency = 3;

	bool bSpatialServiceRunning;
	bool bSpatialServiceInProjectDirectory;

	bool bStartingSpatialService;
	bool bStoppingSpatialService;
};
