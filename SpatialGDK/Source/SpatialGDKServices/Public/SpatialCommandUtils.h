// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "HAL/PlatformProcess.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialCommandUtils, Log, All);

class SpatialCommandUtils
{
public:
	SPATIALGDKSERVICES_API static bool SpatialVersion(bool bIsRunningInChina, const FString& DirectoryToRun, FString& OutResult,
													  int32& OutExitCode);
	SPATIALGDKSERVICES_API static bool AttemptSpatialAuth(bool bIsRunningInChina);
	SPATIALGDKSERVICES_API static bool StartSpatialService(const FString& Version, const FString& RuntimeIP, bool bIsRunningInChina,
														   const FString& DirectoryToRun, FString& OutResult, int32& OutExitCode);
	SPATIALGDKSERVICES_API static bool StopSpatialService(bool bIsRunningInChina, const FString& DirectoryToRun, FString& OutResult,
														  int32& OutExitCode);
	SPATIALGDKSERVICES_API static bool BuildWorkerConfig(bool bIsRunningInChina, const FString& DirectoryToRun, FString& OutResult,
														 int32& OutExitCode);
	SPATIALGDKSERVICES_API static FProcHandle LocalWorkerReplace(const FString& ServicePort, const FString& OldWorker,
																 const FString& NewWorker, bool bIsRunningInChina, uint32* OutProcessID);
	SPATIALGDKSERVICES_API static bool GenerateDevAuthToken(bool bIsRunningInChina, FString& OutTokenSecret, FText& OutErrorMessage);
	SPATIALGDKSERVICES_API static bool HasDevLoginTag(const FString& DeploymentName, bool bIsRunningInChina, FText& OutErrorMessage);
	SPATIALGDKSERVICES_API static FProcHandle StartLocalReceptionistProxyServer(bool bIsRunningInChina, const FString& CloudDeploymentName,
																				const FString& ListeningAddress,
																				const int32 ReceptionistPort, FString& OutResult,
																				int32& OutExitCode);
	SPATIALGDKSERVICES_API static void StopLocalReceptionistProxyServer(FProcHandle& ProcessHandle);
	SPATIALGDKSERVICES_API static bool GetProcessInfoFromPort(int32 Port, FString& OutPid, FString& OutState, FString& OutProcessName);
	SPATIALGDKSERVICES_API static bool GetProcessName(const FString& PID, FString& OutProcessName);
	SPATIALGDKSERVICES_API static bool TryKillProcessWithPID(const FString& PID);
};
