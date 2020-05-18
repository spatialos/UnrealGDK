// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

class SPATIALGDKSERVICES_API ILocalDeploymentManagerInterface
{
public:
	ILocalDeploymentManagerInterface() : bLocalDeploymentRunning(false), bStartingDeployment(false), bStoppingDeployment(false) {}
	virtual ~ILocalDeploymentManagerInterface() {};

	virtual void PreInit(bool bChinaEnabled) = 0;

	virtual void Init(FString RuntimeIPToExpose) = 0;

	virtual bool CheckIfPortIsBound(int32 Port) = 0;
	virtual bool KillProcessBlockingPort(int32 Port) = 0;
	virtual bool LocalDeploymentPreRunChecks() = 0;

	using LocalDeploymentCallback = TFunction<void(bool)>;

	virtual void TryStartLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs, FString SnapshotName, FString RuntimeIPToExpose, const LocalDeploymentCallback& CallBack) = 0;
	virtual bool TryStopLocalDeployment() = 0;

	virtual bool IsLocalDeploymentRunning() const = 0;
	virtual bool GetLocalDeploymentStatus() = 0;

	virtual bool CanStartLocalDeployment() const = 0;
	virtual bool CanStopLocalDeployment() const = 0;

	virtual bool IsDeploymentStarting() const = 0;
	virtual bool IsDeploymentStopping() const = 0;

	virtual bool IsRedeployRequired() const = 0;
	virtual void SetRedeployRequired() = 0;

	// Helper function to inform a client or server whether it should wait for a local deployment to become active.
	virtual bool ShouldWaitForDeployment() const = 0;

	virtual void SetAutoDeploy(bool bAutoDeploy) = 0;

	FSimpleMulticastDelegate OnSpatialShutdown;
	FSimpleMulticastDelegate OnDeploymentStart;

protected:
	virtual bool FinishLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs, FString SnapshotName, FString RuntimeIPToExpose) = 0;

	bool bLocalDeploymentManagerEnabled = true;

	bool bLocalDeploymentRunning;

	bool bStartingDeployment;
	bool bStoppingDeployment;

	FString ExposedRuntimeIP;

	FString LocalRunningDeploymentID;

	bool bRedeployRequired = false;
	bool bAutoDeploy = false;
	bool bIsInChina = false;
};
