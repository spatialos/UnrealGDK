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
	const FString TaskListCmd = TEXT("tasklist");

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

	UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("%s"), *LOCTEXT("FailedToGetBlockingProcessName", "Failed to get the name of the process that is blocking the required port.").ToString());
	
	return ProcessName;
}

bool FLocalReceptionistProxyServerManager::CheckIfPortIsBound(int32 Port)
{
	const FString NetStatCmd = TEXT("netstat");

	// -a display active tcp/udp connections, -o include PID for each connection, -n don't resolve hostnames
	const FString NetStatArgs = TEXT("-n -o -a");
	FString NetStatResult;
	int32 ExitCode;
	FString StdErr;
	bool bSuccess = FPlatformProcess::ExecProcess(*NetStatCmd, *NetStatArgs, &ExitCode, &NetStatResult, &StdErr);

	if (ExitCode == ExitCodeSuccess && bSuccess)
	{
		// Get the line of the netstat output that contains the port we're looking for.
		FRegexPattern PidMatcherPattern(FString::Printf(TEXT("(.*?:%i.)(.*)( [0-9]+)"), Port));
		FRegexMatcher PidMatcher(PidMatcherPattern, NetStatResult);
		if (PidMatcher.FindNext())
		{
			const FString State = PidMatcher.GetCaptureGroup(2 /* Get the State of the process, which is the second group. */);
			if (State.Contains("LISTENING"))
			{
				
				BlockingProcess.Pid = PidMatcher.GetCaptureGroup(3 /* Get the PID, which is the third group. */);
				BlockingProcess.Name = GetProcessName();

				const FText WarningMsg = FText::Format(LOCTEXT("ProcessBlockingPort", "Process {0} with PID : {1} is blocking required port."), FText::FromString(BlockingProcess.Name), FText::FromString(BlockingProcess.Pid));
				UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("%s"), *WarningMsg.ToString());
				
				return true;
			}
		}
	}
	else
	{
		const FText ErrorMsg = FText::Format(LOCTEXT("ProcessBlockingPort", "Failed to check if any process is blocking required port. Error: {0}"), FText::FromString(StdErr));
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("%s"), *ErrorMsg.ToString());
	
	}

	
	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s"), *LOCTEXT("NoProcessBlockingProxyPort", "No Process is blocking the required port.").ToString());

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
		const FText ErrorMessage = FText::Format(LOCTEXT("FailedToKillBlockingProcess", "Failed to kill process blocking required port. Error: '{0}'"), FText::FromString(StdErr));
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("%s"),*ErrorMessage.ToString());
	}

	return bSuccess;
}

bool FLocalReceptionistProxyServerManager::LocalReceptionistProxyServerPreRunChecks(int32 ReceptionistPort)
{
	FText LogMessage;
	//Check if any process is blocking the receptionist port
	if(CheckIfPortIsBound(ReceptionistPort))
	{
		//Try killing the process that blocks the receptionist port 
		bool bProcessKilled = TryKillBlockingPortProcess();
		if (!bProcessKilled)
		{
			LogMessage = FText::Format(LOCTEXT("FailedToKillBlockingPortProcess", "Failed to kill the process '{0}' that is blocking the port."), FText::FromString(BlockingProcess.Name));
			UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("%s"), *LogMessage.ToString());
			return false;
		}

		LogMessage = FText::Format(LOCTEXT("SucceededToKillBlockingPortProcess", "Succesfully killed {0} process that was blocking {1} port."), FText::FromString(BlockingProcess.Name), ReceptionistPort);
		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s"), *LogMessage.ToString());
	}

	return true;
}

void FLocalReceptionistProxyServerManager::Init(int32 Port)
{
	if (!IsRunningCommandlet())
	{
		LocalReceptionistProxyServerPreRunChecks(Port);
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



bool FLocalReceptionistProxyServerManager::TryStartReceptionistProxyServer(bool bIsRunningInChina, const FString& CloudDeploymentName, const FString& ListeningAddress, const int32 ReceptionistPort)
{
	FString StartResult;
	int32 ExitCode;
	bool bSuccess = false;

	//Do not restart the same proxy if you have already a proxy running for the same cloud deployment
	if (bProxyIsRunning && ProxyServerProcHandle.IsValid() && RunningCloudDeploymentName == CloudDeploymentName)
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s"), *LOCTEXT("ServerProxyAlreadyRunning", "The local receptionist proxy server is already running!").ToString());

		return true;
	}

	//Stop receptionist proxy server if it is for a different cloud deployment
	if (bProxyIsRunning && ProxyServerProcHandle.IsValid())
	{
		if(!TryStopReceptionistProxyServer())
		{
			UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s"),*LOCTEXT("FailedToStopPreviousServerProxy", "Failed to stop previous proxy server!").ToString());
		
			return false;
		}

		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s"),*LOCTEXT("SucceededToStopPreviousServerProxy", "Stopped previous proxy server!").ToString());
	}

	bool bProxyStartSuccess = false;
	ProxyServerProcHandle = SpatialCommandUtils::StartLocalReceptionistProxyServer(bIsRunningInChina, CloudDeploymentName, ListeningAddress, ReceptionistPort, StartResult, ExitCode, bProxyStartSuccess);

	//check if process run successfully
	bSuccess = ProxyServerProcHandle.IsValid();
	if (!bSuccess || !bProxyStartSuccess)
	{
		const FText WarningMessage = FText::Format(LOCTEXT("FailedToStartProxyServer", "Starting the local receptionist proxy server failed. Error Code: {0}, Error Message: {1}"), ExitCode, FText::FromString(StartResult));
		UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("%s"), *WarningMessage.ToString());
		return false;
	}

	RunningCloudDeploymentName = CloudDeploymentName;
	bProxyIsRunning = true;

	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("%s"),*LOCTEXT("SuccesfullyStartedServerProxy", "Local receptionist proxy server started sucessfully!").ToString());

	return true;
}
