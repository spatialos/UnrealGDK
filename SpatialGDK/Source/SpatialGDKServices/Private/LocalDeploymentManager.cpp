// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalDeploymentManager.h"

#include "AssetRegistryModule.h"
#include "Async/Async.h"
#include "DirectoryWatcherModule.h"
#include "Editor.h"
#include "FileCache.h"
#include "GeneralProjectSettings.h"
#include "Interop/Connection/EditorWorkerController.h"
#include "Json/Public/Dom/JsonObject.h"
#include "SpatialGDKServicesModule.h"
#include "Misc/MessageDialog.h"
#include "Internationalization/Internationalization.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "UObject/CoreNet.h"
#include "Sockets.h"

DEFINE_LOG_CATEGORY(LogSpatialDeploymentManager);

#define LOCTEXT_NAMESPACE "FLocalDeploymentManager"

static const FString SpatialServiceVersion(TEXT("20190930.180414.3b04a59226"));

FLocalDeploymentManager::FLocalDeploymentManager()
	: bLocalDeploymentRunning(false)
	, bSpatialServiceRunning(false)
	, bSpatialServiceInProjectDirectory(false)
	, bStartingDeployment(false)
	, bStoppingDeployment(false)
	, bStartingSpatialService(false)
	, bStoppingSpatialService(false)
{
#if PLATFORM_WINDOWS
	// Don't kick off background processes when running commandlets
	if (IsRunningCommandlet() == false)
	{
		// Check for the existence of Spatial and Spot. If they don't exist then don't start any background processes. Disable spatial networking if either is true.
		if (!FSpatialGDKServicesModule::SpatialPreRunChecks())
		{
			UE_LOG(LogSpatialDeploymentManager, Warning, TEXT("Pre run checks for LocalDeploymentManager failed. Local deployments cannot be started. Spatial networking will be disabled."));
			GetMutableDefault<UGeneralProjectSettings>()->bSpatialNetworking = false;
			return;
		}

		// Ensure the worker.jsons are up to date.
		WorkerBuildConfigAsync();

		// Watch the worker config directory for changes.
		StartUpWorkerConfigDirectoryWatcher();

		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
		{
			// Stop existing spatial service to guarantee that any new existing spatial service would be running in the current project.
			TryStopSpatialService();

			// Start spatial service in the current project if spatial networking is enabled
			if (GetDefault<UGeneralProjectSettings>()->bSpatialNetworking)
			{
				TryStartSpatialService();
			}
			else
			{
				UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("SpatialOS daemon not started because spatial networking is disabled."));
			}

			// Ensure we have an up to date state of the spatial service and local deployment.
			RefreshServiceStatus();
		});
	}
#endif
}

void FLocalDeploymentManager::StartUpWorkerConfigDirectoryWatcher()
{
	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	if (IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get())
	{
		// Watch the worker config directory for changes.
		const FString SpatialDirectory = FSpatialGDKServicesModule::GetSpatialOSDirectory();
		FString WorkerConfigDirectory = FPaths::Combine(SpatialDirectory, TEXT("workers"));

		if (FPaths::DirectoryExists(WorkerConfigDirectory))
		{
			WorkerConfigDirectoryChangedDelegate = IDirectoryWatcher::FDirectoryChanged::CreateRaw(this, &FLocalDeploymentManager::OnWorkerConfigDirectoryChanged);
			DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(WorkerConfigDirectory, WorkerConfigDirectoryChangedDelegate, WorkerConfigDirectoryChangedDelegateHandle);
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Worker config directory does not exist! Please ensure you have your worker configurations at %s"), *WorkerConfigDirectory);
		}
	}
}

void FLocalDeploymentManager::OnWorkerConfigDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
{
	UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Worker config files updated. Regenerating worker descriptors ('spatial worker build build-config')."));
	WorkerBuildConfigAsync();
}

void FLocalDeploymentManager::WorkerBuildConfigAsync()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		FString BuildConfigArgs = TEXT("worker build build-config");
		FString WorkerBuildConfigResult;
		int32 ExitCode;
		FSpatialGDKServicesModule::ExecuteAndReadOutput(FSpatialGDKServicesModule::GetSpatialExe(), BuildConfigArgs, FSpatialGDKServicesModule::GetSpatialOSDirectory(), WorkerBuildConfigResult, ExitCode);

		if (ExitCode == ExitCodeSuccess)
		{
			UE_LOG(LogSpatialDeploymentManager, Display, TEXT("Building worker configurations succeeded!"));
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Building worker configurations failed. Please ensure your .worker.json files are correct. Result: %s"), *WorkerBuildConfigResult);
		}
	});
}

void FLocalDeploymentManager::RefreshServiceStatus()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		IsServiceRunningAndInCorrectDirectory();
		GetLocalDeploymentStatus();

		// Timers must be started on the game thread.
		AsyncTask(ENamedThreads::GameThread, [this]
		{
			// It's possible that GEditor won't exist when shutting down.
			if (GEditor != nullptr)
			{
				// Start checking for the service status.
				FTimerHandle RefreshTimer;
				GEditor->GetTimerManager()->SetTimer(RefreshTimer, [this]()
				{
					RefreshServiceStatus();
				}, RefreshFrequency, false);
			}
		});
	});
}

bool FLocalDeploymentManager::CheckIfPortIsBound(int32 Port)
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	bool bSuccess = false;
	// Set our broadcast address
	TSharedRef<FInternetAddr> BroadcastAddr = SocketSubsystem->CreateInternetAddr();
	BroadcastAddr->SetBroadcastAddress();
	BroadcastAddr->SetPort(Port);
	// Now the listen address
	TSharedRef<FInternetAddr> ListenAddr = SocketSubsystem->GetLocalBindAddr(*GLog);
	ListenAddr->SetPort(Port);
	// Now create and set up our sockets
	FSocket* ListenSocket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("LAN beacon"), false);
	if (ListenSocket != NULL)
	{
		ListenSocket->SetReuseAddr();
		ListenSocket->SetNonBlocking();
		ListenSocket->SetRecvErr();
		// Bind to our listen port
		if (ListenSocket->Bind(*ListenAddr))
		{
			bSuccess = ListenSocket->Listen(30);
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to bind listen socket to addr (%s) for LAN beacon"), *ListenAddr->ToString(true));
		}
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to create listen socket for LAN beacon"));
	}
	bool bSuccessfullyBoundPort = bSuccess && ListenSocket;
	if (ListenSocket != NULL) {
		// Kill the blocking thing we just opened.
		ListenSocket->Close();
	}
	/* Either we couldn't create the socket or couldn't listen on it so the port is probably not bound */
	return !bSuccessfullyBoundPort;
}

bool FLocalDeploymentManager::PreStartCheck()
{
	const int runtimePort = 5301;
	// Check for the known runtime port (5301) which could be blocked.
	if (CheckIfPortIsBound(runtimePort))
	{
		// If it exists offer the user the ability to kill it.
		if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("KillPortBlockingProcess", "A required port is blocked by another process (potentially by an old deployment). Would you like to kill this process?")) == EAppReturnType::Yes)
		{
			const FString NetStatCmd = FString::Printf(TEXT("netstat"));
			FString Args = TEXT("-n -o -a");
			FString StdOut;
			int32 ExitCode;
			FString StdErr;
			bool bSuccess = FPlatformProcess::ExecProcess(*NetStatCmd, *Args, &ExitCode, &StdOut, &StdErr);

			if (ExitCode == ExitCodeSuccess && bSuccess)
			{
				// Find which line of the output contains the port we're looking for
				int portIndex = StdOut.Find(FString::Printf(TEXT(":%i"), runtimePort));
				// Throw away all lines before
				FString portLine = StdOut.Mid(portIndex);
				int portLineEnd = portLine.Find("\r\n");
				// Throw away all lines after
				portLine = portLine.Mid(0, portLineEnd);

				// Match the PID
				const FRegexPattern pidMatch(" +[0-9]+$");
				FRegexMatcher RegMatcher(pidMatch, portLine);
				if (RegMatcher.FindNext())
				{
					int32 pidIndex = RegMatcher.GetMatchBeginning();
					if (pidIndex >= 0)
					{
						// Throw away everything except the PID
						FString pid = portLine.RightChop(pidIndex);
						pid = pid.Trim();
						const FString TaskkillCmd = FString::Printf(TEXT("taskkill /F /PID %s"), *pid);
						Args = "";
						bSuccess = FPlatformProcess::ExecProcess(*TaskkillCmd, *Args, &ExitCode, &StdOut, &StdErr);
						if (bSuccess && ExitCode == ExitCodeSuccess)
						{
							return true;
						}
						UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Failed to kill process blocking required port"));
					}
				}
			}
		}
		return false;
	}

	return true;
}

bool FLocalDeploymentManager::TryStartLocalDeployment(FString LaunchConfig, FString LaunchArgs, FString SnapshotName)
{
	bRedeployRequired = false;

	if (bStoppingDeployment)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Local deployment is in the process of stopping. New deployment will start when previous one has stopped."));
		while (bStoppingDeployment)
		{
			FPlatformProcess::Sleep(0.1f);
		}
	}

	if (bLocalDeploymentRunning)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Tried to start a local deployment but one is already running."));
		return false;
	}

	bool bPassChecks = PreStartCheck();

	if (!bPassChecks)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Tried to start a local deployment but a required port is already bound by another process."));
		return false;
	}

	LocalRunningDeploymentID.Empty();

	bStartingDeployment = true;

	// If the service is not running then start it.
	if (!bSpatialServiceRunning)
	{
		TryStartSpatialService();
	}

	SnapshotName.RemoveFromEnd(TEXT(".snapshot"));
	FString SpotCreateArgs = FString::Printf(TEXT("alpha deployment create --launch-config=\"%s\" --name=localdeployment --project-name=%s --json --starting-snapshot-id=\"%s\" %s"), *LaunchConfig, *FSpatialGDKServicesModule::GetProjectName(), *SnapshotName, *LaunchArgs);

	FDateTime SpotCreateStart = FDateTime::Now();

	FString SpotCreateResult;
	FString StdErr;
	int32 ExitCode;
	FPlatformProcess::ExecProcess(*FSpatialGDKServicesModule::GetSpotExe(), *SpotCreateArgs, &ExitCode, &SpotCreateResult, &StdErr);
	bStartingDeployment = false;

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Creation of local deployment failed. Result: %s - Error: %s"), *SpotCreateResult, *StdErr);
		return false;
	}

	bool bSuccess = false;

	TSharedPtr<FJsonObject> SpotJsonResult;
	bool bParsingSuccess = FSpatialGDKServicesModule::ParseJson(SpotCreateResult, SpotJsonResult);
	if (!bParsingSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot create result failed. Result: %s"), *SpotCreateResult);
	}

	const TSharedPtr<FJsonObject>* SpotJsonContent = nullptr;
	if (bParsingSuccess && !SpotJsonResult->TryGetObjectField(TEXT("content"), SpotJsonContent))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'content' does not exist in Json result from 'spot create': %s"), *SpotCreateResult);
		bParsingSuccess = false;
	}

	FString DeploymentStatus;
	if (bParsingSuccess && SpotJsonContent->Get()->TryGetStringField(TEXT("status"), DeploymentStatus))
	{
		if (DeploymentStatus == TEXT("RUNNING"))
		{
			FString DeploymentID = SpotJsonContent->Get()->GetStringField(TEXT("id"));
			LocalRunningDeploymentID = DeploymentID;
			bLocalDeploymentRunning = true;

			FDateTime SpotCreateEnd = FDateTime::Now();
			FTimespan Span = SpotCreateEnd - SpotCreateStart;

			OnDeploymentStart.Broadcast();

			UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Successfully created local deployment in %f seconds."), Span.GetTotalSeconds());
			bSuccess = true;
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Local deployment creation failed. Deployment status: %s"), *DeploymentStatus);
		}
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'status' does not exist in Json result from 'spot create': %s"), *SpotCreateResult);
	}

	return bSuccess;
}

bool FLocalDeploymentManager::TryStopLocalDeployment()
{
	if (!bLocalDeploymentRunning || LocalRunningDeploymentID.IsEmpty())
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Tried to stop local deployment but no active deployment exists."));
		return false;
	}

	bStoppingDeployment = true;

	FString SpotDeleteArgs = FString::Printf(TEXT("alpha deployment delete --id=%s --json"), *LocalRunningDeploymentID);

	FString SpotDeleteResult;
	FString StdErr;
	int32 ExitCode;
	FPlatformProcess::ExecProcess(*FSpatialGDKServicesModule::GetSpotExe(), *SpotDeleteArgs, &ExitCode, &SpotDeleteResult, &StdErr);
	bStoppingDeployment = false;

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to stop local deployment! Result: %s - Error: %s"), *SpotDeleteResult, *StdErr);
	}

	bool bSuccess = false;

	TSharedPtr<FJsonObject> SpotJsonResult;
	bool bParsingSuccess = FSpatialGDKServicesModule::ParseJson(SpotDeleteResult, SpotJsonResult);
	if (!bParsingSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot delete result failed. Result: %s"), *SpotDeleteResult);
	}

	const TSharedPtr<FJsonObject>* SpotJsonContent = nullptr;
	if (bParsingSuccess && !SpotJsonResult->TryGetObjectField(TEXT("content"), SpotJsonContent))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'content' does not exist in Json result from 'spot delete': %s"), *SpotDeleteResult);
		bParsingSuccess = false;
	}

	FString DeploymentStatus;
	if (bParsingSuccess && SpotJsonContent->Get()->TryGetStringField(TEXT("status"), DeploymentStatus))
	{
		if (DeploymentStatus == TEXT("STOPPED"))
		{
			UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Successfully stopped local deplyoment"));
			LocalRunningDeploymentID.Empty();
			bLocalDeploymentRunning = false;
			bSuccess = true;
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Stopping local deployment failed. Deployment status: %s"), *DeploymentStatus);
		}
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'status' does not exist in Json result from 'spot delete': %s"), *SpotDeleteResult);
	}

	return bSuccess;
}

bool FLocalDeploymentManager::TryStartSpatialService()
{
	if (bSpatialServiceRunning)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Tried to start spatial service but it is already running."));
		return false;
	}

	bStartingSpatialService = true;

	FString SpatialServiceStartArgs = FString::Printf(TEXT("service start --version=%s"), *SpatialServiceVersion);
	FString ServiceStartResult;
	int32 ExitCode;

	FSpatialGDKServicesModule::ExecuteAndReadOutput(FSpatialGDKServicesModule::GetSpatialExe(), SpatialServiceStartArgs, FSpatialGDKServicesModule::GetSpatialOSDirectory(), ServiceStartResult, ExitCode);

	bStartingSpatialService = false;

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Spatial service failed to start! %s"), *ServiceStartResult);
		return false;
	}

	if (ServiceStartResult.Contains(TEXT("RUNNING")))
	{
		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Spatial service started!"));
		bSpatialServiceRunning = true;
		return true;
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Spatial service failed to start! %s"), *ServiceStartResult);
		bSpatialServiceRunning = false;
		bLocalDeploymentRunning = false;
		return false;
	}
}

bool FLocalDeploymentManager::TryStopSpatialService()
{
	bStoppingSpatialService = true;

	FString SpatialServiceStartArgs = TEXT("service stop");
	FString ServiceStopResult;
	int32 ExitCode;

	FSpatialGDKServicesModule::ExecuteAndReadOutput(FSpatialGDKServicesModule::GetSpatialExe(), SpatialServiceStartArgs, FSpatialGDKServicesModule::GetSpatialOSDirectory(), ServiceStopResult, ExitCode);
	bStoppingSpatialService = false;

	if (ExitCode == ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Spatial service stopped!"));
		bSpatialServiceRunning = false;
		bSpatialServiceInProjectDirectory = true;
		bLocalDeploymentRunning = false;
		return true;
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Spatial service failed to stop! %s"), *ServiceStopResult);
	}

	return false;
}

bool FLocalDeploymentManager::GetLocalDeploymentStatus()
{
	if (!bSpatialServiceRunning)
	{
		bLocalDeploymentRunning = false;
		return bLocalDeploymentRunning;
	}

	FString SpotListArgs = FString::Printf(TEXT("alpha deployment list --project-name=%s --json --view BASIC --status-filter NOT_STOPPED_DEPLOYMENTS"), *FSpatialGDKServicesModule::GetProjectName());

	FString SpotListResult;
	FString StdErr;
	int32 ExitCode;
	FPlatformProcess::ExecProcess(*FSpatialGDKServicesModule::GetSpotExe(), *SpotListArgs, &ExitCode, &SpotListResult, &StdErr);

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to check local deployment status. Result: %s - Error: %s"), *SpotListResult, *StdErr);
		return false;
	}

	TSharedPtr<FJsonObject> SpotJsonResult;
	bool bParsingSuccess = FSpatialGDKServicesModule::ParseJson(SpotListResult, SpotJsonResult);

	if (!bParsingSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot list result failed. Result: %s"), *SpotListResult);
	}

	const TSharedPtr<FJsonObject>* SpotJsonContent = nullptr;
	if (bParsingSuccess && SpotJsonResult->TryGetObjectField(TEXT("content"), SpotJsonContent))
	{
		const TArray<TSharedPtr<FJsonValue>>* JsonDeployments;
		if (!SpotJsonContent->Get()->TryGetArrayField(TEXT("deployments"), JsonDeployments))
		{
			UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("No local deployments running."));
			return false;
		}

		for (TSharedPtr<FJsonValue> JsonDeployment : *JsonDeployments)
		{
			FString DeploymentStatus;
			if (JsonDeployment->AsObject()->TryGetStringField(TEXT("status"), DeploymentStatus))
			{
				if (DeploymentStatus == TEXT("RUNNING"))
				{
					FString DeploymentId = JsonDeployment->AsObject()->GetStringField(TEXT("id"));

					UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Running deployment found: %s"), *DeploymentId);

					LocalRunningDeploymentID = DeploymentId;
					bLocalDeploymentRunning = true;
					return true;
				}
			}
		}
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot list result failed. Can't check deployment status."));
	}

	LocalRunningDeploymentID.Empty();
	bLocalDeploymentRunning = false;
	return false;
}

bool FLocalDeploymentManager::IsServiceRunningAndInCorrectDirectory()
{
	FString SpotProjectInfoArgs = TEXT("alpha service project-info --json");
	FString SpotProjectInfoResult;
	FString StdErr;
	int32 ExitCode;

	FPlatformProcess::ExecProcess(*FSpatialGDKServicesModule::GetSpotExe(), *SpotProjectInfoArgs, &ExitCode, &SpotProjectInfoResult, &StdErr);

	if (ExitCode != ExitCodeSuccess)
	{
		if (ExitCode == ExitCodeNotRunning)
		{
			UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Spatial service is not running: %s"), *SpotProjectInfoResult);
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to get spatial service project info: %s"), *SpotProjectInfoResult);
		}

		bSpatialServiceInProjectDirectory = false;
		bSpatialServiceRunning = false;
		bLocalDeploymentRunning = false;
		return false;
	}

	TSharedPtr<FJsonObject> SpotJsonResult;
	bool bParsingSuccess = FSpatialGDKServicesModule::ParseJson(SpotProjectInfoResult, SpotJsonResult);
	if (!bParsingSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot project info result failed. Result: %s"), *SpotProjectInfoResult);
	}

	const TSharedPtr<FJsonObject>* SpotJsonContent = nullptr;
	if (bParsingSuccess && !SpotJsonResult->TryGetObjectField(TEXT("content"), SpotJsonContent))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'content' does not exist in Json result from 'spot service project-info': %s"), *SpotProjectInfoResult);
		bParsingSuccess = false;
	}

	FString SpatialServiceProjectPath;
	// Get the project file path and ensure it matches the one for the currently running project.
	if (bParsingSuccess && SpotJsonContent->Get()->TryGetStringField(TEXT("projectFilePath"), SpatialServiceProjectPath))
	{
		FString CurrentProjectSpatialPath = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("spatialos.json"));
		FPaths::NormalizeDirectoryName(SpatialServiceProjectPath);
		FPaths::RemoveDuplicateSlashes(SpatialServiceProjectPath);

		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Spatial service running at path: %s "), *SpatialServiceProjectPath);

		if (CurrentProjectSpatialPath.Equals(SpatialServiceProjectPath, ESearchCase::IgnoreCase))
		{
			bSpatialServiceInProjectDirectory = true;
			bSpatialServiceRunning = true;
			return true;
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error,
				TEXT("Spatial service running in a different project! Please run 'spatial service stop' if you wish to launch deployments in the current project. Service at: %s"), *SpatialServiceProjectPath);

			bSpatialServiceInProjectDirectory = false;
			bSpatialServiceRunning = false;
			bLocalDeploymentRunning = false;
			return false;
		}
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'status' does not exist in Json result from 'spot service project-info': %s"), *SpotProjectInfoResult);
	}

	return false;
}

bool FLocalDeploymentManager::IsLocalDeploymentRunning() const
{
	return bLocalDeploymentRunning;
}

bool FLocalDeploymentManager::IsSpatialServiceRunning() const
{
	return bSpatialServiceRunning;
}

bool FLocalDeploymentManager::IsDeploymentStarting() const
{
	return bStartingDeployment;
}

bool FLocalDeploymentManager::IsDeploymentStopping() const
{
	return bStoppingDeployment;
}

bool FLocalDeploymentManager::IsServiceStarting() const
{
	return bStartingSpatialService;
}

bool FLocalDeploymentManager::IsServiceStopping() const
{
	return bStoppingSpatialService;
}

bool FLocalDeploymentManager::IsRedeployRequired() const
{
	return bRedeployRequired;
}

void FLocalDeploymentManager::SetRedeployRequired()
{
	bRedeployRequired = true;
}

bool FLocalDeploymentManager::ShouldWaitForDeployment() const
{
	if (bAutoDeploy)
	{
		return !IsLocalDeploymentRunning() || IsDeploymentStopping() || IsDeploymentStarting();
	}
	else
	{
		return false;
	}
}

void FLocalDeploymentManager::SetAutoDeploy(bool bInAutoDeploy)
{
	bAutoDeploy = bInAutoDeploy;
}
