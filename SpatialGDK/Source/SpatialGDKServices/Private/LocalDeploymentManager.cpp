// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalDeploymentManager.h"

#include "AssetRegistryModule.h"
#include "Async/Async.h"
#include "DirectoryWatcherModule.h"
#include "Editor.h"
#include "FileCache.h"
#include "GeneralProjectSettings.h"
#include "IPAddress.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Regex.h"
#include "Json/Public/Dom/JsonObject.h"
#include "Misc/MessageDialog.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "SpatialCommandUtils.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "UObject/CoreNet.h"

DEFINE_LOG_CATEGORY(LogSpatialDeploymentManager);

#define LOCTEXT_NAMESPACE "FLocalDeploymentManager"

static const FString SpatialServiceVersion(TEXT("20200611.170527.924b1f1c45"));

FLocalDeploymentManager::FLocalDeploymentManager()
	: bLocalDeploymentRunning(false)
	, bSpatialServiceRunning(false)
	, bSpatialServiceInProjectDirectory(false)
	, bStartingDeployment(false)
	, bStoppingDeployment(false)
	, bStartingSpatialService(false)
	, bStoppingSpatialService(false)
{
}

void FLocalDeploymentManager::PreInit(bool bChinaEnabled)
{
	bIsInChina = bChinaEnabled;
	// Don't kick off background processes when running commandlets
	const bool bCommandletRunning = IsRunningCommandlet();

	// Check for the existence of Spatial and Spot. If they don't exist then don't start any background processes.
	const bool bSpatialServicesAvailable = FSpatialGDKServicesModule::SpatialPreRunChecks(bIsInChina);

	if (bCommandletRunning || !bSpatialServicesAvailable)
	{
		if (!bSpatialServicesAvailable)
		{
			UE_LOG(LogSpatialDeploymentManager, Warning,
				   TEXT("Pre run checks for LocalDeploymentManager failed. Local deployments cannot be started."));
		}
		bLocalDeploymentManagerEnabled = false;
		return;
	}

	// Ensure the worker.jsons are up to date.
	WorkerBuildConfigAsync();

	// Watch the worker config directory for changes.
	StartUpWorkerConfigDirectoryWatcher();
}

void FLocalDeploymentManager::Init(FString RuntimeIPToExpose)
{
	if (bLocalDeploymentManagerEnabled)
	{
		// If a service was running, restart to guarantee that the service is running in this project with the correct settings.
		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("(Re)starting Spatial service in this project."));

		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, RuntimeIPToExpose] {
			// Stop existing spatial service to guarantee that any new existing spatial service would be running in the current project.
			TryStopSpatialService();
			// Start spatial service in the current project if spatial networking is enabled

			if (GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking())
			{
				TryStartSpatialService(RuntimeIPToExpose);
			}
			else
			{
				UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("SpatialOS daemon not started because spatial networking is disabled."));
			}

			// Ensure we have an up to date state of the spatial service and local deployment.
			RefreshServiceStatus();
		});
	}
}

void FLocalDeploymentManager::StartUpWorkerConfigDirectoryWatcher()
{
	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	if (IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get())
	{
		// Watch the worker config directory for changes.
		FString WorkerConfigDirectory = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("workers"));

		if (FPaths::DirectoryExists(WorkerConfigDirectory))
		{
			WorkerConfigDirectoryChangedDelegate =
				IDirectoryWatcher::FDirectoryChanged::CreateRaw(this, &FLocalDeploymentManager::OnWorkerConfigDirectoryChanged);
			DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(WorkerConfigDirectory, WorkerConfigDirectoryChangedDelegate,
																	  WorkerConfigDirectoryChangedDelegateHandle);
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error,
				   TEXT("Worker config directory does not exist! Please ensure you have your worker configurations at %s"),
				   *WorkerConfigDirectory);
		}
	}
}

void FLocalDeploymentManager::OnWorkerConfigDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
{
	UE_LOG(LogSpatialDeploymentManager, Log,
		   TEXT("Worker config files updated. Regenerating worker descriptors ('spatial worker build build-config')."));
	WorkerBuildConfigAsync();
}

void FLocalDeploymentManager::WorkerBuildConfigAsync()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this] {
		FString WorkerBuildConfigResult;
		int32 ExitCode;
		bool bSuccess = SpatialCommandUtils::BuildWorkerConfig(bIsInChina, SpatialGDKServicesConstants::SpatialOSDirectory,
															   WorkerBuildConfigResult, ExitCode);

		if (bSuccess)
		{
			UE_LOG(LogSpatialDeploymentManager, Display, TEXT("Building worker configurations succeeded!"));
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error,
				   TEXT("Building worker configurations failed. Please ensure your .worker.json files are correct. Result: %s"),
				   *WorkerBuildConfigResult);
		}
	});
}

void FLocalDeploymentManager::RefreshServiceStatus()
{
	if (!bLocalDeploymentManagerEnabled)
	{
		return;
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this] {
		IsServiceRunningAndInCorrectDirectory();
		GetLocalDeploymentStatus();

		// Timers must be started on the game thread.
		AsyncTask(ENamedThreads::GameThread, [this] {
			// It's possible that GEditor won't exist when shutting down.
			if (GEditor != nullptr)
			{
				// Start checking for the service status.
				FTimerHandle RefreshTimer;
				GEditor->GetTimerManager()->SetTimer(
					RefreshTimer,
					[this]() {
						RefreshServiceStatus();
					},
					RefreshFrequency, false);
			}
		});
	});
}

bool FLocalDeploymentManager::CheckIfPortIsBound(int32 Port)
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	bool bCanBindToPort = false;

	// Set our broadcast address.
	TSharedRef<FInternetAddr> BroadcastAddr = SocketSubsystem->CreateInternetAddr();
	BroadcastAddr->SetBroadcastAddress();
	BroadcastAddr->SetPort(Port);

	// Now the listen address.
	TSharedRef<FInternetAddr> ListenAddr = SocketSubsystem->GetLocalBindAddr(*GLog);
	ListenAddr->SetPort(Port);
	const FString SocketName = TEXT("Runtime Port Test");

	// Now create and set up our sockets.
	FSocket* ListenSocket = SocketSubsystem->CreateSocket(NAME_Stream, SocketName, false /* bForceUDP */);
	if (ListenSocket != nullptr)
	{
		ListenSocket->SetReuseAddr();
		ListenSocket->SetNonBlocking();
		ListenSocket->SetRecvErr();

		// Bind to our listen port.
		if (ListenSocket->Bind(*ListenAddr))
		{
			bCanBindToPort = ListenSocket->Listen(0 /* MaxBacklog*/);
			ListenSocket->Close();
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Verbose,
				   TEXT("Failed to bind listen socket to addr (%s) for %s, the port is likely in use"), *ListenAddr->ToString(true),
				   *SocketName);
		}
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Failed to create listen socket for %s, the port is likely in use"), *SocketName);
	}

	// Either we couldn't create the socket or couldn't listen on it so the port is probably bound.
	return !bCanBindToPort;
}

bool FLocalDeploymentManager::KillProcessBlockingPort(int32 Port)
{
	FString PID;
	FString State;
	FString ProcessName;

	bool bSuccess = SpatialCommandUtils::GetProcessInfoFromPort(Port, PID, State, ProcessName);
	if (bSuccess)
	{
		bSuccess = SpatialCommandUtils::TryKillProcessWithPID(PID);
	}

	return bSuccess;
}

bool FLocalDeploymentManager::LocalDeploymentPreRunChecks()
{
	bool bSuccess = true;

	// Check for the known runtime port (5301) which could be blocked.
	if (CheckIfPortIsBound(RequiredRuntimePort))
	{
		// If it exists offer the user the ability to kill it.
		if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("KillPortBlockingProcess",
															 "A required port is blocked by another process (potentially by an old "
															 "deployment). Would you like to kill this process?"))
			== EAppReturnType::Yes)
		{
			bSuccess = KillProcessBlockingPort(RequiredRuntimePort);
		}
		else
		{
			bSuccess = false;
		}
	}

	if (!bSpatialServiceInProjectDirectory && bSpatialServiceRunning)
	{
		if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("StopSpatialServiceFromDifferentProject",
															 "An instance of the SpatialOS Runtime is running with another project. Would "
															 "you like to stop it and start the Runtime for this project?"))
			== EAppReturnType::Yes)
		{
			bSuccess = TryStopSpatialService();
		}
		else
		{
			bSuccess = false;
		}
	}

	return bSuccess;
}

bool FLocalDeploymentManager::FinishLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs, FString SnapshotName,
													FString RuntimeIPToExpose)
{
	FString SpotCreateArgs =
		FString::Printf(TEXT("alpha deployment create --launch-config=\"%s\" --name=localdeployment --project-name=%s --json "
							 "--starting-snapshot-id=\"%s\" --runtime-version=%s %s"),
						*LaunchConfig, *FSpatialGDKServicesModule::GetProjectName(), *SnapshotName, *RuntimeVersion, *LaunchArgs);

	FDateTime SpotCreateStart = FDateTime::Now();

	FString SpotCreateResult;
	FString StdErr;
	int32 ExitCode;
	FPlatformProcess::ExecProcess(*SpatialGDKServicesConstants::SpotExe, *SpotCreateArgs, &ExitCode, &SpotCreateResult, &StdErr);
	bStartingDeployment = false;

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Creation of local deployment failed. Result: %s - Error: %s"), *SpotCreateResult,
			   *StdErr);
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
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'content' does not exist in Json result from 'spot create': %s"),
			   *SpotCreateResult);
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

			AsyncTask(ENamedThreads::GameThread, [this] {
				OnDeploymentStart.Broadcast();
			});

			UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Successfully created local deployment in %f seconds."), Span.GetTotalSeconds());
			bSuccess = true;
		}
		else
		{
			UE_LOG(
				LogSpatialDeploymentManager, Error,
				TEXT("Local deployment creation failed. Deployment status: %s. Please check the 'Spatial Output' window for more details."),
				*DeploymentStatus);
		}
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'status' does not exist in Json result from 'spot create': %s"),
			   *SpotCreateResult);
	}

	return true;
}

void FLocalDeploymentManager::TryStartLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs,
													  FString SnapshotName, FString RuntimeIPToExpose,
													  const LocalDeploymentCallback& CallBack)
{
	if (!bLocalDeploymentManagerEnabled)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose,
			   TEXT("Local deployment manager is disabled because spatial services are unavailable."));
		if (CallBack)
		{
			CallBack(false);
		}
		return;
	}

	bRedeployRequired = false;

	if (bStoppingDeployment)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose,
			   TEXT("Local deployment is in the process of stopping. New deployment will start when previous one has stopped."));
		while (bStoppingDeployment)
		{
			FPlatformProcess::Sleep(0.1f);
		}
	}

	if (bLocalDeploymentRunning)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Tried to start a local deployment but one is already running."));
		if (CallBack)
		{
			CallBack(false);
		}
		return;
	}

	if (!LocalDeploymentPreRunChecks())
	{
		UE_LOG(LogSpatialDeploymentManager, Error,
			   TEXT("Tried to start a local deployment but a required port is already bound by another process."));
		if (CallBack)
		{
			CallBack(false);
		}
		return;
	}

	LocalRunningDeploymentID.Empty();

	bStartingDeployment = true;

	// Stop the currently running service if the runtime IP is to be exposed, but is different from the one specified
	if (ExposedRuntimeIP != RuntimeIPToExpose)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose,
			   TEXT("Settings for exposing runtime IP have changed since service startup. Restarting service to reflect changes."));
		TryStopSpatialService();
	}

	// If the service is not running then start it.
	if (!bSpatialServiceRunning)
	{
		TryStartSpatialService(RuntimeIPToExpose);
	}

	SnapshotName.RemoveFromEnd(TEXT(".snapshot"));

	AttemptSpatialAuthResult = Async(
		EAsyncExecution::Thread,
		[this]() {
			return SpatialCommandUtils::AttemptSpatialAuth(bIsInChina);
		},
		[this, LaunchConfig, RuntimeVersion, LaunchArgs, SnapshotName, RuntimeIPToExpose, CallBack]() {
			bool bSuccess = AttemptSpatialAuthResult.IsReady() && AttemptSpatialAuthResult.Get() == true;
			if (bSuccess)
			{
				bSuccess = FinishLocalDeployment(LaunchConfig, RuntimeVersion, LaunchArgs, SnapshotName, RuntimeIPToExpose);
			}
			else
			{
				UE_LOG(LogSpatialDeploymentManager, Error,
					   TEXT("Failed to authenticate against SpatialOS while attempting to start a local deployment."));
			}
			bStartingDeployment = false;

			if (CallBack)
			{
				CallBack(bSuccess);
			}
		});

	return;
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
	FPlatformProcess::ExecProcess(*SpatialGDKServicesConstants::SpotExe, *SpotDeleteArgs, &ExitCode, &SpotDeleteResult, &StdErr);
	bStoppingDeployment = false;

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to stop local deployment! Result: %s - Error: %s"), *SpotDeleteResult,
			   *StdErr);
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
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'content' does not exist in Json result from 'spot delete': %s"),
			   *SpotDeleteResult);
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
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'status' does not exist in Json result from 'spot delete': %s"),
			   *SpotDeleteResult);
	}

	return bSuccess;
}

bool FLocalDeploymentManager::TryStartSpatialService(FString RuntimeIPToExpose)
{
	if (!bLocalDeploymentManagerEnabled)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose,
			   TEXT("Local deployment manager is disabled because spatial services are unavailable."));
		return false;
	}

	if (bSpatialServiceRunning)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Tried to start spatial service but it is already running."));
		return false;
	}
	else if (bStartingSpatialService)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Tried to start spatial service but it is already being started."));
		return false;
	}

	bStartingSpatialService = true;

	FString ServiceStartResult;
	int32 ExitCode;
	bool bSuccess = SpatialCommandUtils::StartSpatialService(*SpatialServiceVersion, *RuntimeIPToExpose, bIsInChina,
															 SpatialGDKServicesConstants::SpatialOSDirectory, ServiceStartResult, ExitCode);

	bStartingSpatialService = false;

	if (bSuccess && ServiceStartResult.Contains(TEXT("RUNNING")))
	{
		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Spatial service started!"));
		ExposedRuntimeIP = RuntimeIPToExpose;
		bSpatialServiceRunning = true;
		return true;
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Spatial service failed to start! %s"), *ServiceStartResult);
		ExposedRuntimeIP = TEXT("");
		bSpatialServiceRunning = false;
		bLocalDeploymentRunning = false;
		return false;
	}
}

bool FLocalDeploymentManager::TryStopSpatialService()
{
	if (!bLocalDeploymentManagerEnabled)
	{
		return false;
	}

	if (bStoppingSpatialService)
	{
		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Tried to stop spatial service but it is already being stopped."));
		return false;
	}

	bStoppingSpatialService = true;

	FString ServiceStopResult;
	int32 ExitCode;
	bool bSuccess =
		SpatialCommandUtils::StopSpatialService(bIsInChina, SpatialGDKServicesConstants::SpatialOSDirectory, ServiceStopResult, ExitCode);

	bStoppingSpatialService = false;

	if (bSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Spatial service stopped!"));
		ExposedRuntimeIP = TEXT("");
		bSpatialServiceRunning = false;
		bSpatialServiceInProjectDirectory = true;
		bLocalDeploymentRunning = false;
		return true;
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

	FString SpotListArgs =
		FString::Printf(TEXT("alpha deployment list --project-name=%s --json --view BASIC --status-filter NOT_STOPPED_DEPLOYMENTS"),
						*FSpatialGDKServicesModule::GetProjectName());

	FString SpotListResult;
	FString StdErr;
	int32 ExitCode;
	FPlatformProcess::ExecProcess(*SpatialGDKServicesConstants::SpotExe, *SpotListArgs, &ExitCode, &SpotListResult, &StdErr);

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to check local deployment status. Result: %s - Error: %s"), *SpotListResult,
			   *StdErr);
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
	if (!bLocalDeploymentManagerEnabled)
	{
		return false;
	}

	FString SpotProjectInfoArgs = TEXT("alpha service project-info --json");
	FString SpotProjectInfoResult;
	FString StdErr;
	int32 ExitCode;

	FPlatformProcess::ExecProcess(*SpatialGDKServicesConstants::SpotExe, *SpotProjectInfoArgs, &ExitCode, &SpotProjectInfoResult, &StdErr);

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
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot project info result failed. Result: %s"),
			   *SpotProjectInfoResult);
	}

	const TSharedPtr<FJsonObject>* SpotJsonContent = nullptr;
	if (bParsingSuccess && !SpotJsonResult->TryGetObjectField(TEXT("content"), SpotJsonContent))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'content' does not exist in Json result from 'spot service project-info': %s"),
			   *SpotProjectInfoResult);
		bParsingSuccess = false;
	}

	FString SpatialServiceProjectPath;
	// Get the project file path and ensure it matches the one for the currently running project.
	if (bParsingSuccess && SpotJsonContent->Get()->TryGetStringField(TEXT("projectFilePath"), SpatialServiceProjectPath))
	{
		FString CurrentProjectSpatialPath = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSDirectory, TEXT("spatialos.json"));
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
				   TEXT("Spatial service running in a different project! Please run 'spatial service stop' if you wish to start "
						"deployments in the current project. Service at: %s"),
				   *SpatialServiceProjectPath);

			ExposedRuntimeIP = TEXT("");
			bSpatialServiceInProjectDirectory = false;
			bSpatialServiceRunning = false;
			bLocalDeploymentRunning = false;
			return false;
		}
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'status' does not exist in Json result from 'spot service project-info': %s"),
			   *SpotProjectInfoResult);
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

#undef LOCTEXT_NAMESPACE
