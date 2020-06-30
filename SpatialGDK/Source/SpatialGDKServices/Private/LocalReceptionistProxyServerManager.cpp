// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalReceptionistProxyServerManager.h"

#include "Internationalization/Regex.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "UObject/CoreNet.h"

#include "SpatialCommandUtils.h"
#include "SpatialGDKServicesConstants.h"
#include "..\Public\LocalReceptionistProxyServerManager.h"

DEFINE_LOG_CATEGORY(LogLocalReceptionistProxyServerManager);

#define LOCTEXT_NAMESPACE "FLocalReceptionistProxyServerManager"

namespace
{
	static const FString ProxyInfoFilePath = FPaths::Combine(SpatialGDKServicesConstants::ProxyFileDirectory, SpatialGDKServicesConstants::ProxyInfoFilename);
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
	if (ExitCode == 0 && bSuccess)
	{
		FRegexPattern ProcessNamePattern(TEXT("\"(.+?)\""));
		FRegexMatcher ProcessNameMatcher(ProcessNamePattern, TaskListResult);
		if (ProcessNameMatcher.FindNext())
		{
			OutProcessName = ProcessNameMatcher.GetCaptureGroup(1 /* Get the Name of the process, which is the first group. */);

			return true;
		}
	}

	UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Failed to get the name of the process that is blocking the required port."));
	
	return false;
}

bool FLocalReceptionistProxyServerManager::CheckIfPortIsBound(int32 Port, FString& OutPID, FString& OutLogMsg)
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
				OutLogMsg = TEXT("%s process with PID: %s"), ProcessName, OutPID;
				return true;
			}

			OutLogMsg = TEXT("Unknown process with PID: %s."), *OutPID;

			return true;
		}
	}
	OutLogMsg = TEXT("No Process is blocking the required port.");

	return false;
}

bool FLocalReceptionistProxyServerManager::LocalReceptionistProxyServerPreRunChecks(int32 ReceptionistPort)
{
	FString OutLogMessage;
	FString PID;

	// Check if any process is blocking the receptionist port
	if (CheckIfPortIsBound(ReceptionistPort, PID, OutLogMessage))
	{
		// Try killing the process that blocks the receptionist port if the process blocking the port is a previously running proxy.
		if (PID == ParsePid())
		{
			bool bProcessKilled = SpatialCommandUtils::TryKillProcessWithPID(PID);
			if (!bProcessKilled)
			{
				UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Failed to kill the process that is blocking the port. %s"), *OutLogMessage);
				return false;
			}

			UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Succesfully killed %s"), *OutLogMessage);
			return true;
		}

		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("The required port is blocked from a different process with Pid: %s"), *PID);
		return false;
	}

	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("The required port is not blocked!"));
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


TSharedPtr<FJsonObject> FLocalReceptionistProxyServerManager::ParsePidFile()
{
	FString ProxyInfoFileResult;
	TSharedPtr<FJsonObject> JsonParsedProxyInfoFile;

	if (FFileHelper::LoadFileToString(ProxyInfoFileResult, *ProxyInfoFilePath))
	{
		if (FSpatialGDKServicesModule::ParseJson(ProxyInfoFileResult, JsonParsedProxyInfoFile))
		{
			return JsonParsedProxyInfoFile;
		}
		else
		{
			UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Json parsing of proxyInfo.json failed. Can't get proxy's Pid."));
		}
	}
	else
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Loading proxyInfo.json failed. Can't get proxy's Pid."));
	}

	return nullptr;
}

void FLocalReceptionistProxyServerManager::SetPidInJson(const FString& Pid)
{
	FString ProxyInfoFileResult;

	// If file does not exist create an empty json file 
	if (!FPaths::FileExists(ProxyInfoFilePath))
	{
		FFileHelper::SaveStringToFile(TEXT("{ }"), *ProxyInfoFilePath);
	}

	TSharedPtr<FJsonObject> JsonParsedProxyInfoFile = ParsePidFile();
	if (!JsonParsedProxyInfoFile.IsValid())
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to update Pid(%s). Please ensure that the following file exists: %s"), *Pid, *SpatialGDKServicesConstants::ProxyInfoFilename);
		return;
	}

	JsonParsedProxyInfoFile->SetStringField("pid", Pid);

	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&ProxyInfoFileResult);
	if (!FJsonSerializer::Serialize(JsonParsedProxyInfoFile.ToSharedRef(), JsonWriter))
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to write pid to parsed proxy info file. Unable to serialize content to json file."));
		return;
	}
	if (!FFileHelper::SaveStringToFile(ProxyInfoFileResult, *FPaths::Combine(SpatialGDKServicesConstants::ProxyFileDirectory, SpatialGDKServicesConstants::ProxyInfoFilename)))
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to write file content to %s"), *SpatialGDKServicesConstants::ProxyInfoFilename);
	}
}

FString FLocalReceptionistProxyServerManager::ParsePid()
{
	FString Pid;

	if (TSharedPtr<FJsonObject> JsonParsedProxyInfoFile = ParsePidFile())
	{
		if (JsonParsedProxyInfoFile->TryGetStringField(TEXT("pid"), Pid))
		{
			return Pid;
		}
		else
		{
			UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("'pid' does not exist in proxyInfo.json. Can't read proxy's Pid."));
		}
	}

	Pid.Empty();
	return Pid;
}

bool FLocalReceptionistProxyServerManager::TryStartReceptionistProxyServer(bool bIsRunningInChina, const FString& CloudDeploymentName, const FString& ListeningAddress, const int32 ReceptionistPort)
{
	FString StartResult;
	int32 ExitCode;

	// Do not restart the same proxy if you have already a proxy running for the same cloud deployment
	if (bProxyIsRunning && ProxyServerProcHandle.IsValid() && RunningCloudDeploymentName == CloudDeploymentName && RunningProxyListeningAddress == ListeningAddress && RunningProxyReceptionistPort == ReceptionistPort)
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("The local receptionist proxy server is already running!"));

		return true;
	}

	// Stop receptionist proxy server if it is for a different cloud deployment, port or listening address
	if (bProxyIsRunning && ProxyServerProcHandle.IsValid())
	{
		if (!TryStopReceptionistProxyServer())
		{
			UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Failed to stop previous proxy server!"));
		
			return false;
		}

		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Stopped previous proxy server sucessfully!"));
	}

	ProxyServerProcHandle = SpatialCommandUtils::StartLocalReceptionistProxyServer(bIsRunningInChina, CloudDeploymentName, ListeningAddress, ReceptionistPort, StartResult, ExitCode);

	// Check if process run successfully
	if (!ProxyServerProcHandle.IsValid())
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Starting the local receptionist proxy server failed. Error Code: %s, Error Message: %s"), ExitCode, *StartResult);
		return false;
	}

	FString Pid;
	FString State;

	// Save the server receptionist proxy process's Pid in a Json file
	if (SpatialCommandUtils::GetProcessInfoFromPort(ReceptionistPort, Pid, State))
	{
		SetPidInJson(Pid);
	}

	RunningCloudDeploymentName = CloudDeploymentName;
	RunningProxyListeningAddress = ListeningAddress;
	RunningProxyReceptionistPort = ReceptionistPort;
	bProxyIsRunning = true;

	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Local receptionist proxy server started sucessfully!"));

	return true;
}
