// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLocalReceptionistProxyServerManager, Log, All);

class FLocalReceptionistProxyServerManager
{
public:
	FLocalReceptionistProxyServerManager();

	bool CheckIfPortIsBound(uint16 Port, FString& OutPID, FString& OutLogMessages);
	bool LocalReceptionistProxyServerPreRunChecks(uint16 ReceptionistPort);

	void SPATIALGDKSERVICES_API Init(uint16 ReceptionistPort);
	bool SPATIALGDKSERVICES_API TryStartReceptionistProxyServer(bool bIsRunningInChina, const FString& CloudDeploymentName,
																const FString& ListeningAddress, uint16 ReceptionistPort);
	bool SPATIALGDKSERVICES_API TryStopReceptionistProxyServer();

private:
	static TSharedPtr<FJsonObject> ParsePIDFile();
	static void SavePIDInJson(const FString& PID);
	static bool GetPreviousReceptionistProxyPID(FString& OutPID);
	static void DeletePIDFile();

	FProcHandle ProxyServerProcHandle;
	FString RunningCloudDeploymentName;
	FString RunningProxyListeningAddress;
	uint16 RunningProxyReceptionistPort;

	bool bProxyIsRunning = false;
};
