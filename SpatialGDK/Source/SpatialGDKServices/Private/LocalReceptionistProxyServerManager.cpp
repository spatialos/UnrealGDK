// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalReceptionistProxyServerManager.h"

#include "Internationalization/Regex.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "UObject/CoreNet.h"

#include "SpatialCommandUtils.h"

DEFINE_LOG_CATEGORY(LogLocalReceptionistProxyServerManager);

#define LOCTEXT_NAMESPACE "FLocalReceptionistProxyServerManager"

namespace
{
	static const int32 ExitCodeSuccess = 0;
}

FLocalReceptionistProxyServerManager::FLocalReceptionistProxyServerManager()
: RunningCloudDeploymentName(TEXT(""))
{
}

bool FLocalReceptionistProxyServerManager::GetProcessName(const FString& PID, FString& OutProcessName)
{
	bool bSuccess = false;
	OutProcessName = TEXT("");
	const FString TaskListCmd = TEXT("tasklist");

	// Get the task list line for the process with Pid 
	const FString TaskListArgs = FString::Printf(TEXT(" /fi \"PID eq %s\" /nh /fo:csv"), *PID);
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
			OutProcessName = ProcessNameMatcher.GetCaptureGroup(1 /* Get the Name of the process, which is the first group. */);

			return true;
		}
	}

	UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("%s"), *LOCTEXT("FailedToGetBlockingProcessName", "Failed to get the name of the process that is blocking the required port.").ToString());
	
	return false;
}

bool FLocalReceptionistProxyServerManager::CheckIfPortIsBound(int32 Port, FString& OutPID, FText& OutLogMsg)
{
	FString State;
	FString ProcessName;
	bool bSuccess = SpatialCommandUtils::GetProcessInfoFromPort(Port, OutPID, State);
	if (bSuccess)
	{
		if (State.Contains("LISTENING"))
		{
			if (GetProcessName(OutPID, ProcessName))
			{
				OutLogMsg = FText::Format(LOCTEXT("ProcessBlockingPort", "{0} process with PID:{1}."), FText::FromString(ProcessName), FText::FromString(OutPID));
				return true;
			}

			OutLogMsg = FText::Format(LOCTEXT("ProcessBlockingPort", "Unknown process with PID:{1}."), FText::FromString(OutPID));

			return true;
		}
	}
	OutLogMsg = LOCTEXT("NoProcessBlockingProxyPort", "No Process is blocking the required port.");

	return false;
}

bool FLocalReceptionistProxyServerManager::LocalReceptionistProxyServerPreRunChecks(int32 ReceptionistPort)
{
	FText OutLogMessage;
	FString PID;

	// Check if any process is blocking the receptionist port
	if (CheckIfPortIsBound(ReceptionistPort, PID, OutLogMessage))
	{
		// Try killing the process that blocks the receptionist port 
		bool bProcessKilled = SpatialCommandUtils::TryKillProcessWithPID(PID);
		if (!bProcessKilled)
		{
			UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("%s %s!"), *LOCTEXT("FailedToKillBlockingPortProcess", "Failed to kill the process that is blocking the port.").ToString(),*OutLogMessage.ToString());
			return false;
		}

		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s %s."), *LOCTEXT("KilledBlockingPortProcess", "Succesfully killed").ToString(), *OutLogMessage.ToString());
		return true;
	}

	OutLogMessage = LOCTEXT("NoProcessBlockingPort", "The required port is not blocked!");
	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s"), *OutLogMessage.ToString());
	return true;
}

void FLocalReceptionistProxyServerManager::Init(int32 Port)
{
	if (!IsRunningCommandlet())
	{
		LocalReceptionistProxyServerPreRunChecks(Port);
	}
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



bool FLocalReceptionistProxyServerManager::TryStartReceptionistProxyServer(bool bIsRunningInChina, const FString& CloudDeploymentName, const FString& ListeningAddress, const int32 ReceptionistPort)
{
	FString StartResult;
	int32 ExitCode;

	// Do not restart the same proxy if you have already a proxy running for the same cloud deployment
	if (bProxyIsRunning && ProxyServerProcHandle.IsValid() && RunningCloudDeploymentName == CloudDeploymentName && RunningProxyListeningAddress == ListeningAddress && RunningProxyReceptionistPort == ReceptionistPort)
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s"), *LOCTEXT("ServerProxyAlreadyRunning", "The local receptionist proxy server is already running!").ToString());

		return true;
	}

	// Stop receptionist proxy server if it is for a different cloud deployment, port or listening address
	if (bProxyIsRunning && ProxyServerProcHandle.IsValid())
	{
		if (!TryStopReceptionistProxyServer())
		{
			UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s"),*LOCTEXT("FailedToStopPreviousServerProxy", "Failed to stop previous proxy server!").ToString());
		
			return false;
		}

		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s"),*LOCTEXT("SucceededToStopPreviousServerProxy", "Stopped previous proxy server!").ToString());
	}

	ProxyServerProcHandle = SpatialCommandUtils::StartLocalReceptionistProxyServer(bIsRunningInChina, CloudDeploymentName, ListeningAddress, ReceptionistPort, StartResult, ExitCode);

	// Check if process run successfully
	if (!ProxyServerProcHandle.IsValid())
	{
		const FText WarningMessage = FText::Format(LOCTEXT("FailedToStartProxyServer", "Starting the local receptionist proxy server failed. Error Code: {0}, Error Message: {1}"), ExitCode, FText::FromString(StartResult));
		UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("%s"), *WarningMessage.ToString());
		return false;
	}

	RunningCloudDeploymentName = CloudDeploymentName;
	RunningProxyListeningAddress = ListeningAddress;
	RunningProxyReceptionistPort = ReceptionistPort;
	bProxyIsRunning = true;

	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s"),*LOCTEXT("SuccesfullyStartedServerProxy", "Local receptionist proxy server started sucessfully!").ToString());

	return true;
}
