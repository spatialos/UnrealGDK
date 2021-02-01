// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLocalReceptionistProxyServerManager, Log, All);

class FLocalReceptionistProxyServerManager
{
public:
	FLocalReceptionistProxyServerManager();

	bool CheckIfPortIsBound(int32 Port, FString& OutPID, FString& OutLogMessages);
	bool LocalReceptionistProxyServerPreRunChecks(int32 ReceptionistPort);

	void SPATIALGDKSERVICES_API Init(int32 ReceptionistPort);
	bool SPATIALGDKSERVICES_API TryStartReceptionistProxyServer(bool bIsRunningInChina, const FString& CloudDeploymentName,
																const FString& ListeningAddress, int32 ReceptionistPort);
	bool SPATIALGDKSERVICES_API TryStopReceptionistProxyServer();

private:
	static TSharedPtr<FJsonObject> ParsePIDFile();
	static void SavePIDInJson(const FString& PID);
	static bool GetPreviousReceptionistProxyPID(FString& OutPID);
	static void DeletePIDFile();

	FProcHandle ProxyServerProcHandle;
	FString RunningCloudDeploymentName;
	int32 RunningProxyReceptionistPort;
	FString RunningProxyListeningAddress;

	bool bProxyIsRunning = false;
};
