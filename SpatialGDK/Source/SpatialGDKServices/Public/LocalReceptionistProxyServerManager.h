// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLocalReceptionistProxyServerManager, Log, All);

class FLocalReceptionistProxyServerManager
{
public:
	FLocalReceptionistProxyServerManager();

	bool CheckIfPortIsBound(int32 Port);
	bool TryKillBlockingPortProcess();
	bool LocalReceptionistProxyServerPreRunChecks();

	FString GetProcessName();

	void SPATIALGDKSERVICES_API Init();
	bool SPATIALGDKSERVICES_API TryStartReceptionistProxyServer(bool bIsRunningInChina, const FString& CloudDeploymentName);
	bool SPATIALGDKSERVICES_API TryStopReceptionistProxyServer();
	
private:

	struct BlockingPortProcess
	{
		FString Pid = TEXT("");
		FString Name = TEXT("");
	};

	BlockingPortProcess BlockingProcess;

	FProcHandle ProxyServerProcHandle;
	FString RunningCloudDeploymentName;

	static const int32 ExitCodeSuccess = 0;
	static const int32 ReceptionistPort = 7777;
	const FString ListeningAddress = TEXT("127.0.0.1");

	bool bProxyIsRunning = false;
};
