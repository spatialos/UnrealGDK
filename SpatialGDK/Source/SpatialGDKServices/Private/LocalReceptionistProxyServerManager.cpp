// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalReceptionistProxyServerManager.h"

#include "Internationalization/Regex.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "SpatialCommandUtils.h"
#include "UObject/CoreNet.h"

DEFINE_LOG_CATEGORY(LogLocalReceptionistProxyServerManager);

#define LOCTEXT_NAMESPACE "FLocalReceptionistProxyServerManager"

FLocalReceptionistProxyServerManager::FLocalReceptionistProxyServerManager()
: RunningCloudDeploymentName("")
{
}

FString FLocalReceptionistProxyServerManager::GetProcessName()
{
	bool bSuccess = false;
	FString ProcessName = "";
	const FString TaskListCmd = FString::Printf(TEXT("tasklist"));

	// get the task list line for the process with Pid 
	const FString TaskListArgs = FString::Printf(TEXT(" /fi \"PID eq %s\" /nh /fo:csv"), *BlockingProcess.Pid);
	FString TaskListResult;
	int32 ExitCode;
	FString StdErr;
	bSuccess = FPlatformProcess::ExecProcess(*TaskListCmd, *TaskListArgs, &ExitCode, &TaskListResult, &StdErr);
	if (ExitCode == ExitCodeSuccess && bSuccess)
	{
		FRegexPattern ProcessNamePattern(TEXT("\"(.+?)\""));
		FRegexMatcher ProcessNameMatcher(ProcessNamePattern, TaskListResult);
		if (ProcessNameMatcher.FindNext())
		{
			ProcessName = ProcessNameMatcher.GetCaptureGroup(1 /* Get the Name of the process, which is the first group. */);

			return ProcessName;
		}
	}

	UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Failed to get the name of the process that is blocking the required port."));
	
	return ProcessName;
}

bool FLocalReceptionistProxyServerManager::CheckIfPortIsBound()
{
	const FString NetStatCmd = FString::Printf(TEXT("netstat"));

	// -a display active tcp/udp connections, -o include PID for each connection, -n don't resolve hostnames
	const FString NetStatArgs = TEXT("-n -o -a");
	FString NetStatResult;
	int32 ExitCode;
	FString StdErr;
	bool bSuccess = FPlatformProcess::ExecProcess(*NetStatCmd, *NetStatArgs, &ExitCode, &NetStatResult, &StdErr);

	if (ExitCode == ExitCodeSuccess && bSuccess)
	{
		// Get the line of the netstat output that contains the port we're looking for.
		FRegexPattern PidMatcherPattern(FString::Printf(TEXT("(.*?:%i.)(.*)( [0-9]+)"), ReceptionistPort));
		FRegexMatcher PidMatcher(PidMatcherPattern, NetStatResult);
		if (PidMatcher.FindNext())
		{
			const FString State = PidMatcher.GetCaptureGroup(2 /* Get the State of the process, which is the second group. */);
			if (State.Contains("LISTENING"))
			{
				
				BlockingProcess.Pid = PidMatcher.GetCaptureGroup(3 /* Get the PID, which is the third group. */);
				BlockingProcess.Name = GetProcessName();
				
				UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Process %s with PID:%s is blocking required port."), *BlockingProcess.Name, *BlockingProcess.Pid);
				
				return true;
			}
		}
	}
	else
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to check if any process is blocking required port. Error: %s"), *StdErr);
	}

	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("No Process is blocking the runtime port."));

	return false;
}


bool FLocalReceptionistProxyServerManager::TryKillBlockingPortProcess()
{
	bool bSuccess = false;

	int32 ExitCode;
	FString StdErr;

	const FString TaskKillCmd = TEXT("taskkill");
	const FString TaskKillArgs = FString::Printf(TEXT("/F /PID %s"), *BlockingProcess.Pid);
	FString TaskKillResult;
	bSuccess = FPlatformProcess::ExecProcess(*TaskKillCmd, *TaskKillArgs, &ExitCode, &TaskKillResult, &StdErr);
	bSuccess = bSuccess && ExitCode == ExitCodeSuccess;
	if (!bSuccess)
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to kill process blocking required port. Error: %s"), *StdErr);
	}

	return bSuccess;
}

bool FLocalReceptionistProxyServerManager::LocalReceptionistProxyServerPreRunChecks()
{
	//Check if any process is blocking the 7777 port
	if(CheckIfPortIsBound())
	{
		//Try killing the process that blocks the 7777 port 
		bool bProcessKilled = TryKillBlockingPortProcess();
		if (!bProcessKilled)
		{
			UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Failed to kill the process %s that is blocking the port. "), *BlockingProcess.Name);
			return false;
		}

		UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Succesfully killed %s process that was blocking %d port."), *BlockingProcess.Name, ReceptionistPort);
	}

	return true;
}

void FLocalReceptionistProxyServerManager::Init()
{
	if (!IsRunningCommandlet())
	{
		LocalReceptionistProxyServerPreRunChecks();
	}
	return;
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
		if(!TryStopReceptionistProxyServer())
		{
			UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Failed to stop previous proxy server!"))
			return false;
		}

		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Stopped previous proxy server!"));
	}

	bool bProxyStartSuccess = false;
	ProxyServerProcHandle = SpatialCommandUtils::StartLocalReceptionistProxyServer(bIsRunningInChina, CloudDeploymentName, StartResult, ExitCode, bProxyStartSuccess);

	//check if process run successfully
	bSuccess = ProxyServerProcHandle.IsValid();
	if (!bSuccess || !bProxyStartSuccess)
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Starting the local receptionist proxy server failed. Error Code: %d, Error Message: %s"), ExitCode, *StartResult);
		return false;
	}

	RunningCloudDeploymentName = CloudDeploymentName;
	bProxyIsRunning = true;

	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Local receptionist proxy server started sucessfully!"));

	return true;
}
