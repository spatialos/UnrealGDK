// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalDeploymentManager.h"

#include "AssetRegistryModule.h"
#include "Async/Async.h"
#include "DirectoryWatcherModule.h"
#include "Editor.h"
#include "FileCache.h"
#include "GeneralProjectSettings.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Interop/Connection/EditorWorkerController.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "SpatialGDKServicesModule.h"

DEFINE_LOG_CATEGORY(LogSpatialDeploymentManager);

static const FString SpatialExe(TEXT("spatial.exe"));
static const FString SpatialServiceVersion(TEXT("20190716.094149.1b6d448edd"));

FLocalDeploymentManager::FLocalDeploymentManager()
	: bLocalDeploymentRunning(false)
	, bSpatialServiceRunning(false)
	, bSpatialServiceInProjectDirectory(false)
	, bStartingDeployment(false)
	, bStoppingDeployment(false)
	, bStartingSpatialService(false)
	, bStoppingSpatialService(false)
{
	// Get the project name from the spatialos.json.
	ProjectName = GetProjectName();

#if PLATFORM_WINDOWS
	// Don't kick off background processes when running commandlets
	if (IsRunningCommandlet() == false)
	{
		// Ensure the worker.jsons are up to date.
		WorkerBuildConfigAsync();

		// Watch the worker config directory for changes.
		StartUpWorkerConfigDirectoryWatcher();

		// Restart the spatial service so it is guaranteed to be running in the current project.
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
		{
			TryStopSpatialService();
			TryStartSpatialService();

			// Ensure we have an up to date state of the spatial service and local deployment.
			RefreshServiceStatus();
		});
	}
#endif
}

const FString FLocalDeploymentManager::GetSpotExe()
{
	return FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/spot.exe"));
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

FString FLocalDeploymentManager::GetProjectName()
{
	const FString SpatialDirectory = FSpatialGDKServicesModule::GetSpatialOSDirectory();

	FString SpatialFileName = TEXT("spatialos.json");
	FString SpatialFileResult;
	FFileHelper::LoadFileToString(SpatialFileResult, *FPaths::Combine(SpatialDirectory, SpatialFileName));

	TSharedPtr<FJsonObject> JsonParsedSpatialFile;
	if (ParseJson(SpatialFileResult, JsonParsedSpatialFile))
	{
		if (JsonParsedSpatialFile->TryGetStringField(TEXT("name"), ProjectName))
		{
			return ProjectName;
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'name' does not exist in spatialos.json. Can't read project name."));
		}
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spatialos.json failed. Can't get project name."));
	}

	ProjectName.Empty();
	return ProjectName;
}

void FLocalDeploymentManager::WorkerBuildConfigAsync()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		FString BuildConfigArgs = TEXT("worker build build-config");
		FString WorkerBuildConfigResult;
		int32 ExitCode;
		ExecuteAndReadOutput(SpatialExe, BuildConfigArgs, FSpatialGDKServicesModule::GetSpatialOSDirectory(), WorkerBuildConfigResult, ExitCode);

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

bool FLocalDeploymentManager::ParseJson(const FString& RawJsonString, TSharedPtr<FJsonObject>& JsonParsed)
{
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(RawJsonString);
	return FJsonSerializer::Deserialize(JsonReader, JsonParsed);
}

// ExecuteAndReadOutput exists so that a spatial command window does not spawn when using 'spatial.exe'. It does not however allow reading from StdErr.
// For other processes which do not spawn cmd windows, use ExecProcess instead.
void FLocalDeploymentManager::ExecuteAndReadOutput(const FString& Executable, const FString& Arguments, const FString& DirectoryToRun, FString& OutResult, int32& ExitCode)
{
#if PLATFORM_WINDOWS
	UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Executing '%s' with arguments '%s' in directory '%s'"), *Executable, *Arguments, *DirectoryToRun);

	void* ReadPipe = nullptr;
	void* WritePipe = nullptr;
	ensure(FPlatformProcess::CreatePipe(ReadPipe, WritePipe));

	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*Executable, *Arguments, false, true, true, nullptr, 1 /*PriorityModifer*/, *DirectoryToRun, WritePipe);

	if (ProcHandle.IsValid())
	{
		for (bool bProcessFinished = false; !bProcessFinished; )
		{
			bProcessFinished = FPlatformProcess::GetProcReturnCode(ProcHandle, &ExitCode);

			OutResult = OutResult.Append(FPlatformProcess::ReadPipe(ReadPipe));
			FPlatformProcess::Sleep(0.01f);
		}

		FPlatformProcess::CloseProc(ProcHandle);
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Execution failed. '%s' with arguments '%s' in directory '%s' "), *Executable, *Arguments, *DirectoryToRun);
	}

	FPlatformProcess::ClosePipe(0, ReadPipe);
	FPlatformProcess::ClosePipe(0, WritePipe);
#else
	ExitCode = 1;
#endif
}

void FLocalDeploymentManager::RefreshServiceStatus()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		GetServiceStatus();
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

bool FLocalDeploymentManager::TryStartLocalDeployment(FString LaunchConfig, FString LaunchArgs)
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

	LocalRunningDeploymentID.Empty();

	bStartingDeployment = true;

	// If the service is not running then start it.
	if (!bSpatialServiceRunning)
	{
		TryStartSpatialService();
	}

	FString SpotCreateArgs = FString::Printf(TEXT("alpha deployment create --launch-config=\"%s\" --name=localdeployment --project-name=%s --json %s"), *LaunchConfig, *ProjectName, *LaunchArgs);

	FDateTime SpotCreateStart = FDateTime::Now();

	FString SpotCreateResult;
	FString StdErr;
	int32 ExitCode;
	FPlatformProcess::ExecProcess(*GetSpotExe(), *SpotCreateArgs, &ExitCode, &SpotCreateResult, &StdErr);
	bStartingDeployment = false;

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Creation of local deployment failed. Result: %s - Error: %s"), *SpotCreateResult, *StdErr);
		return false;
	}

	bool bSuccess = false;

	TSharedPtr<FJsonObject> SpotJsonResult;
	bool bParsingSuccess = ParseJson(SpotCreateResult, SpotJsonResult);
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
	FPlatformProcess::ExecProcess(*GetSpotExe(), *SpotDeleteArgs, &ExitCode, &SpotDeleteResult, &StdErr);
	bStoppingDeployment = false;

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to stop local deployment! Result: %s - Error: %s"), *SpotDeleteResult, *StdErr);
	}

	bool bSuccess = false;

	TSharedPtr<FJsonObject> SpotJsonResult;
	bool bPasingSuccess = ParseJson(SpotDeleteResult, SpotJsonResult);
	if (!bPasingSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot delete result failed. Result: %s"), *SpotDeleteResult);
	}

	const TSharedPtr<FJsonObject>* SpotJsonContent = nullptr;
	if (bPasingSuccess && !SpotJsonResult->TryGetObjectField(TEXT("content"), SpotJsonContent))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'content' does not exist in Json result from 'spot delete': %s"), *SpotDeleteResult);
		bPasingSuccess = false;
	}

	FString DeploymentStatus;
	if (bPasingSuccess && SpotJsonContent->Get()->TryGetStringField(TEXT("status"), DeploymentStatus))
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
	ExecuteAndReadOutput(SpatialExe, SpatialServiceStartArgs, FSpatialGDKServicesModule::GetSpatialOSDirectory(), ServiceStartResult, ExitCode);

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
	ExecuteAndReadOutput(SpatialExe, SpatialServiceStartArgs, FSpatialGDKServicesModule::GetSpatialOSDirectory(), ServiceStopResult, ExitCode);
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

	FString SpotListArgs = FString::Printf(TEXT("alpha deployment list --project-name=%s --json --view BASIC --status-filter NOT_STOPPED_DEPLOYMENTS"), *ProjectName);

	FString SpotListResult;
	FString StdErr;
	int32 ExitCode;
	FPlatformProcess::ExecProcess(*GetSpotExe(), *SpotListArgs, &ExitCode, &SpotListResult, &StdErr);

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to check local deployment status. Result: %s - Error: %s"), *SpotListResult, *StdErr);
		return false;
	}

	TSharedPtr<FJsonObject> SpotJsonResult;
	bool bPasingSuccess = ParseJson(SpotListResult, SpotJsonResult);
	if (!bPasingSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot list result failed. Result: %s"), *SpotListResult);
	}

	const TSharedPtr<FJsonObject>* SpotJsonContent = nullptr;
	if (bPasingSuccess && SpotJsonResult->TryGetObjectField(TEXT("content"), SpotJsonContent))
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

bool FLocalDeploymentManager::GetServiceStatus()
{
	FString SpatialServiceStatusArgs = TEXT("service status");
	FString ServiceStatusResult;
	int32 ExitCode;
	ExecuteAndReadOutput(SpatialExe, SpatialServiceStatusArgs, FSpatialGDKServicesModule::GetSpatialOSDirectory(), ServiceStatusResult, ExitCode);

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to check spatial service status: %s"), *ServiceStatusResult);
	}

	if (ServiceStatusResult.Contains(TEXT("Local API service is not running.")))
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Spatial service not running."));
		bSpatialServiceInProjectDirectory = true;
		bSpatialServiceRunning = false;
		bLocalDeploymentRunning = false;
		return false;
	}
	else if (ServiceStatusResult.Contains(TEXT("Local API service is running")))
	{
		bSpatialServiceInProjectDirectory = IsServiceInCorrectDirectory(ServiceStatusResult);
		bSpatialServiceRunning = true;
		return true;
	}

	return false;
}

bool FLocalDeploymentManager::IsServiceInCorrectDirectory(const FString& ServiceStatusResult)
{
	// Get the project file path and ensure it matches the one for the currently running project.
	FString SpatialServiceProjectPath;
	if (ServiceStatusResult.Split(TEXT("project file path: "), nullptr, &SpatialServiceProjectPath))
	{
		// Remove the trailing '" cli-version' that comes with non-interactive 'spatial' calls.
		SpatialServiceProjectPath.Split(TEXT("\" cli-version"), &SpatialServiceProjectPath, nullptr);

		FString CurrentProjectSpatialPath = FPaths::Combine(FSpatialGDKServicesModule::GetSpatialOSDirectory(), TEXT("spatialos.json"));
		FPaths::NormalizeDirectoryName(SpatialServiceProjectPath);
		FPaths::RemoveDuplicateSlashes(SpatialServiceProjectPath);

		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Spatial service running at path: %s "), *SpatialServiceProjectPath);

		if (CurrentProjectSpatialPath.Equals(SpatialServiceProjectPath, ESearchCase::IgnoreCase))
		{
			return true;
		}
		else if (SpatialServiceProjectPath.Contains(TEXT("not available")))
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Spatial service has hit an erroneous state! Please run 'spatial service stop'."));
			return false;
		}
		else
		{
			UE_LOG(LogSpatialDeploymentManager, Error,
				TEXT("Spatial service running in a different project! Please run 'spatial service stop' if you wish to launch deployments in the current project. Service at: %s"), *SpatialServiceProjectPath);
			return false;
		}
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
