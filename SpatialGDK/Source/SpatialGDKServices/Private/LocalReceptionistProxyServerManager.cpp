// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalReceptionistProxyServerManager.h"

#include "HAL/PlatformFilemanager.h"
#include "Internationalization/Regex.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "UObject/CoreNet.h"

#include "SpatialCommandUtils.h"
#include "SpatialGDKServicesConstants.h"

DEFINE_LOG_CATEGORY(LogLocalReceptionistProxyServerManager);

#define LOCTEXT_NAMESPACE "FLocalReceptionistProxyServerManager"

FLocalReceptionistProxyServerManager::FLocalReceptionistProxyServerManager()
	: RunningCloudDeploymentName(TEXT(""))
{
}

bool FLocalReceptionistProxyServerManager::CheckIfPortIsBound(int32 Port, FString& OutPID, FString& OutLogMsg)
{
	FString State;
	FString ProcessName;
	bool bSuccess = SpatialCommandUtils::GetProcessInfoFromPort(Port, OutPID, State, ProcessName);
	if (bSuccess && State.Contains("LISTEN"))
	{
		OutLogMsg = FString::Printf(TEXT("%s process with PID: %s"), *ProcessName, *OutPID);
		return true;
	}

	OutLogMsg = TEXT("No Process is blocking the required port or Failed to check if process is blocked.");
	return false;
}

bool FLocalReceptionistProxyServerManager::LocalReceptionistProxyServerPreRunChecks(int32 ReceptionistPort)
{
	FString OutLogMessage;
	FString PID;
	FString PreviousPID;

	// Check if any process is blocking the receptionist port
	if (!CheckIfPortIsBound(ReceptionistPort, PID, OutLogMessage))
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("The required port is not blocked: %s"), *OutLogMessage);
		return true;
	}

	// Get the previous running proxy's PID
	if (GetPreviousReceptionistProxyPID(PreviousPID))
	{
		// Try killing the process that blocks the receptionist port if the process blocking the port is a previously running proxy.
		if (FCString::Atoi(*PID) == FCString::Atoi(*PreviousPID))
		{
			bool bProcessKilled = SpatialCommandUtils::TryKillProcessWithPID(PID);
			if (bProcessKilled)
			{
				UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Successfully killed %s"), *OutLogMessage);
			}
			else
			{
				UE_LOG(LogLocalReceptionistProxyServerManager, Warning, TEXT("Failed to kill the process that is blocking the port. %s"),
					   *OutLogMessage);
			}

			return bProcessKilled;
		}
	}

	UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("The required port is blocked from %s."), *OutLogMessage);
	return false;
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

		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Json parsing of %s failed. Can't get proxy's PID."),
			   *SpatialGDKServicesConstants::ProxyInfoFilePath);
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
		UE_LOG(LogLocalReceptionistProxyServerManager, Error,
			   TEXT("Failed to write PID to parsed proxy info file. Unable toS serialize content to json file."));
		return;
	}
	if (!FFileHelper::SaveStringToFile(ProxyInfoFileResult, *SpatialGDKServicesConstants::ProxyInfoFilePath))
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to write file content to %s"),
			   *SpatialGDKServicesConstants::ProxyInfoFilePath);
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

		UE_LOG(LogLocalReceptionistProxyServerManager, Error,
			   TEXT("Local Receptionist Proxy is running but 'pid' does not exist in %s. Can't read proxy's PID."),
			   *SpatialGDKServicesConstants::ProxyInfoFilePath);
		return false;
	}

	UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Local Receptionist Proxy is not running or the %s file got deleted."),
		   *SpatialGDKServicesConstants::ProxyInfoFilePath);

	OutPID.Empty();
	return false;
}

void FLocalReceptionistProxyServerManager::DeletePIDFile()
{
	if (FPaths::FileExists(SpatialGDKServicesConstants::ProxyInfoFilePath))
	{
		FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*SpatialGDKServicesConstants::ProxyInfoFilePath);
	}
}

bool FLocalReceptionistProxyServerManager::TryStartReceptionistProxyServer(bool bIsRunningInChina, const FString& CloudDeploymentName,
																		   const FString& ListeningAddress, const int32 ReceptionistPort)
{
	FString StartResult;
	int32 ExitCode;

	// Do not start receptionist proxy if the cloud deployment name is not specified
	if (CloudDeploymentName.IsEmpty())
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("No deployment name has been specified."));
		return false;
	}

	// Do not restart the same proxy if you have already a proxy running for the same cloud deployment
	if (bProxyIsRunning && ProxyServerProcHandle.IsValid() && RunningCloudDeploymentName == CloudDeploymentName
		&& RunningProxyListeningAddress == ListeningAddress && RunningProxyReceptionistPort == ReceptionistPort)
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("The local receptionist proxy server is already running!"));

		return true;
	}

	// Stop receptionist proxy server if it is for a different cloud deployment, port or listening address
	if (bProxyIsRunning && ProxyServerProcHandle.IsValid())
	{
		if (!TryStopReceptionistProxyServer())
		{
			UE_LOG(LogLocalReceptionistProxyServerManager, Error, TEXT("Failed to stop previous proxy server!"));
			return false;
		}

		UE_LOG(LogLocalReceptionistProxyServerManager, Log, TEXT("Stopped previous proxy server successfully!"));
	}

	ProxyServerProcHandle = SpatialCommandUtils::StartLocalReceptionistProxyServer(bIsRunningInChina, CloudDeploymentName, ListeningAddress,
																				   ReceptionistPort, StartResult, ExitCode);

	// Check if process run successfully
	if (!ProxyServerProcHandle.IsValid())
	{
		UE_LOG(LogLocalReceptionistProxyServerManager, Error,
			   TEXT("Starting the local receptionist proxy server failed. Error Code: %d, Error Message: %s"), ExitCode, *StartResult);
		ProxyServerProcHandle.Reset();
		return false;
	}

	FString PID;
	FString State;
	FString ProcessName;

	// Save the server receptionist proxy process's PID in a Json file
	if (SpatialCommandUtils::GetProcessInfoFromPort(ReceptionistPort, PID, State, ProcessName))
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
