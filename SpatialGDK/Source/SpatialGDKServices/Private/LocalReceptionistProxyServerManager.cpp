// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalReceptionistProxyServerManager.h"

#include "Internationalization/Regex.h"
#include "SpatialCommandUtils.h"
#include "UObject/CoreNet.h"

DEFINE_LOG_CATEGORY(LogLocalReceptionistProxyServerManager);

#define LOCTEXT_NAMESPACE "FLocalReceptionistProxyServerManager"

FLocalReceptionistProxyServerManager::FLocalReceptionistProxyServerManager()
: RunningCloudDeploymentName("")
{
}

bool FLocalReceptionistProxyServerManager::KillProcessBlockingPort()
{
	bool bSuccess = true;

	const FString NetStatCmd = FString::Printf(TEXT("netstat"));

	// -a display active tcp/udp connections, -o include PID for each connection, -n don't resolve hostnames
	const FString NetStatArgs = TEXT("-n -o -a");
	FString NetStatResult;
	int32 ExitCode;
	FString StdErr;
	bSuccess = FPlatformProcess::ExecProcess(*NetStatCmd, *NetStatArgs, &ExitCode, &NetStatResult, &StdErr);

	if (ExitCode == ExitCodeSuccess && bSuccess)
	{
		// Get the line of the netstat output that contains the port we're looking for.
		FRegexPattern PidMatcherPattern(FString::Printf(TEXT("(.*?:%i.)(.*)( [0-9]+)"), ReceptionistPort));
		FRegexMatcher PidMatcher(PidMatcherPattern, NetStatResult);
		if (PidMatcher.FindNext())
		{
			FString Pid = PidMatcher.GetCaptureGroup(3 /* Get the PID, which is the third group. */);

			const FString TaskKillCmd = TEXT("taskkill");
			const FString TaskKillArgs = FString::Printf(TEXT("/F /PID %s"), *Pid);
			FString TaskKillResult;
			bSuccess = FPlatformProcess::ExecProcess(*TaskKillCmd, *TaskKillArgs, &ExitCode, &TaskKillResult, &StdErr);
			bSuccess = bSuccess && ExitCode == ExitCodeSuccess;
			if (!bSuccess)
			{
				UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to kill process blocking required port. Error: %s"), *StdErr);
			}
		}
		else
		{
			bSuccess = false;
			UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to find PID of the process that is blocking the runtime port."));
		}
	}
	else
	{
		bSuccess = false;
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to find the process that is blocking required port. Error: %s"), *StdErr);
	}

	return bSuccess;

}


bool FLocalReceptionistProxyServerManager::TryStopReceptionistProxyServer()
{
	if (ProxyServerProcHandle.IsValid())
	{
		SpatialCommandUtils::StopLocalReceptionistProxyServer(ProxyServerProcHandle);
		bProxyIsRunning = false;
		return true;
	}

	return false;
}

bool FLocalReceptionistProxyServerManager::TryStartReceptionistProxyServer(bool bIsRunningInChina, const FString& CloudDeploymentName)
{
	FString StartResult;
	int32 ExitCode;
	bool bSuccess = false;

	//Do not restart the same proxy if you have already a proxy running for the same cloud deployment
	if (bProxyIsRunning && ProxyServerProcHandle.IsValid() && RunningCloudDeploymentName == CloudDeploymentName)
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("The local receptionist proxy server is already running!"));
		return true;
	}

	//Stop receptionist proxy server if it is for a different cloud deployment
	if (bProxyIsRunning && ProxyServerProcHandle.IsValid())
	{
		TryStopReceptionistProxyServer();
		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Stop Previous proxy server!"));
	}

	bool bProxyStartSuccess = false;
	//Retry if the receptionist proxy server fails to start
	while (!bProxyStartSuccess)
	{
		//Start the local receptionist proxy server
		ProxyServerProcHandle = SpatialCommandUtils::StartLocalReceptionistProxyServer(bIsRunningInChina, CloudDeploymentName, StartResult, ExitCode);

		//check if process run successfully
		bSuccess = ProxyServerProcHandle.IsValid();
		if (!bSuccess)
		{
			UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Starting the local receptionist proxy server failed. Error Code: %d, Error Message: %s"), ExitCode, *StartResult);
			return false;
		}

		//check if Proxy started successfully
		bProxyStartSuccess = StartResult.Contains("The receptionist proxy is available");
		if(!bProxyStartSuccess)
		{
			//Try killing the process that blocks the 7777 port before retrying to start the receptionist proxy server
			bool bProcessKilled = KillProcessBlockingPort();
			if(!bProcessKilled)
			{
				UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Starting the local receptionist proxy server failed. Error Code: %d, Error Message: %s"), ExitCode, *StartResult);
				return false;
			}
		}
	}

	RunningCloudDeploymentName = CloudDeploymentName;
	bProxyIsRunning = true;

	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Local receptionist proxy server started sucessfully!"));

	return true;
}


