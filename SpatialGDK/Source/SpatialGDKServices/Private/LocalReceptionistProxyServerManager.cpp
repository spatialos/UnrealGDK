// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalReceptionistProxyServerManager.h"

#include "HAL/PlatformFilemanager.h"
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

DEFINE_LOG_CATEGORY(LogLocalReceptionistProxyServerManager);

#define LOCTEXT_NAMESPACE "FLocalReceptionistProxyServerManager"

FLocalReceptionistProxyServerManager::FLocalReceptionistProxyServerManager()
	: RunningCloudDeploymentName(TEXT(""))
{
}

bool FLocalReceptionistProxyServerManager::GetProcessName(const FString& PID, FString& OutProcessName)
{
	bool bSuccess = false;
	OutProcessName = TEXT("");
	const FString TaskListCmd = TEXT("tasklist");

	// Get the task list line for the process with PID 
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
	FString PreviousPID;

	// Check if any process is blocking the receptionist port
	if (CheckIfPortIsBound(ReceptionistPort, PID, OutLogMessage))
	{
		if (GetPreviousReceptionistProxyPID(PreviousPID))
		{
			// Try killing the process that blocks the receptionist port if the process blocking the port is a previously running proxy.
			if (PID == PreviousPID)
			{
				bool bProcessKilled = SpatialCommandUtils::TryKillProcessWithPID(PID);
				if (!bProcessKilled)
				{
					UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Failed to kill the process that is blocking the port. %s"), *OutLogMessage);
					return false;
				}

				UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Successfully killed %s"), *OutLogMessage);
				return true;
			}

			UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("The required port is blocked from a different process with PID: %s"), *PID);
			return false;
		}

		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("The required port is blocked from an unidentified process with PID: %s"), *PID);
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
		ProxyServerProcHandle.Reset();
		bProxyIsRunning = false;
		DeletePIDFile();
		return true;
	}

	return false;
}


TSharedPtr<FJsonObject> FLocalReceptionistProxyServerManager::ParsePIDFile()
{
	FString ProxyInfoFileResult;
	TSharedPtr<FJsonObject> JsonParsedProxyInfoFile;

	if (FFileHelper::LoadFileToString(ProxyInfoFileResult, *SpatialGDKServicesConstants::ProxyInfoFilePath))
	{
		if (FSpatialGDKServicesModule::ParseJson(ProxyInfoFileResult, JsonParsedProxyInfoFile))
		{
			return JsonParsedProxyInfoFile;
		}
		else
		{
			UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Json parsing of %s failed. Can't get proxy's PID."), *SpatialGDKServicesConstants::ProxyInfoFilePath);
		}
	}

	return nullptr;
}

void FLocalReceptionistProxyServerManager::SavePIDInJson(const FString& PID)
{
	FString ProxyInfoFileResult;
	TSharedPtr<FJsonObject> JsonParsedProxyInfoFile = MakeShareable(new FJsonObject());
	JsonParsedProxyInfoFile->SetStringField("pid", PID);

	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&ProxyInfoFileResult);
	if (!FJsonSerializer::Serialize(JsonParsedProxyInfoFile.ToSharedRef(), JsonWriter))
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to write PID to parsed proxy info file. Unable to serialize content to json file."));
		return;
	}
	if (!FFileHelper::SaveStringToFile(ProxyInfoFileResult, *SpatialGDKServicesConstants::ProxyInfoFilePath))
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to write file content to %s"), *SpatialGDKServicesConstants::ProxyInfoFilePath);
	}
}

bool FLocalReceptionistProxyServerManager::GetPreviousReceptionistProxyPID(FString& OutPID)
{
	if (TSharedPtr<FJsonObject> JsonParsedProxyInfoFile = ParsePIDFile())
	{
		if (JsonParsedProxyInfoFile->TryGetStringField(TEXT("pid"), OutPID))
		{
			return true;
		}

		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Local Receptionist Proxy is running but 'pid' does not exist in %s. Can't read proxy's PID."), *SpatialGDKServicesConstants::ProxyInfoFilePath);
		return false;
	}

	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Local Receptionist Proxy is not running."));

	OutPID.Empty();
	return true;
}

void FLocalReceptionistProxyServerManager::DeletePIDFile()
{
	if (FPaths::FileExists(SpatialGDKServicesConstants::ProxyInfoFilePath))
	{
		FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*SpatialGDKServicesConstants::ProxyInfoFilePath);
	}
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

		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Stopped previous proxy server successfully!"));
	}

	ProxyServerProcHandle = SpatialCommandUtils::StartLocalReceptionistProxyServer(bIsRunningInChina, CloudDeploymentName, ListeningAddress, ReceptionistPort, StartResult, ExitCode);

	// Check if process run successfully
	if (!ProxyServerProcHandle.IsValid())
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Starting the local receptionist proxy server failed. Error Code: %s, Error Message: %s"), ExitCode, *StartResult);
		ProxyServerProcHandle.Reset();
		return false;
	}

	FString PID;
	FString State;

	// Save the server receptionist proxy process's PID in a Json file
	if (SpatialCommandUtils::GetProcessInfoFromPort(ReceptionistPort, PID, State))
	{
		SavePIDInJson(PID);
	}

	RunningCloudDeploymentName = CloudDeploymentName;
	RunningProxyListeningAddress = ListeningAddress;
	RunningProxyReceptionistPort = ReceptionistPort;
	bProxyIsRunning = true;

	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Local receptionist proxy server started successfully!"));

	return true;
}
