// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLocalReceptionistProxyServerManager, Log, All);

class FLocalReceptionistProxyServerManager
{
public:
	FLocalReceptionistProxyServerManager();

	bool CheckIfPortIsBound(int32 Port);
	bool KillProcessBlockingPort();
	bool LocalReceptionistProxyServerPreRunChecks();

	void SPATIALGDKSERVICES_API Init();
	bool SPATIALGDKSERVICES_API TryStartReceptionistProxyServer(bool bIsRunningInChina, const FString& CloudDeploymentName);
	bool SPATIALGDKSERVICES_API TryStopReceptionistProxyServer();
	
private:
	FProcHandle ProxyServerProcHandle;
	FString RunningCloudDeploymentName;

	static const int32 ExitCodeSuccess = 0;
	static const int32 ReceptionistPort = 7777;

	bool bProxyIsRunning = false;
};
