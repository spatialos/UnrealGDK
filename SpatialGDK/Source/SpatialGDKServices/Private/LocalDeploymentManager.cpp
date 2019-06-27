// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalDeploymentManager.h"

#include "AssetRegistryModule.h"
#include "Async/Async.h"
#include "DirectoryWatcherModule.h"
#include "FileCache.h"
#include "GeneralProjectSettings.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Interop/Connection/EditorWorkerController.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY(LogSpatialDeploymentManager);

FLocalDeploymentManager::FLocalDeploymentManager()
{
	bLocalDeploymentRunning = false;
	bSpatialServiceRunning = false;

	bStartingDeployment = false;
	bStoppingDeployment = false;

	bStartingSpatialService = false;
	bStoppingSpatialService = false;

	// For checking whether we can stop or start. Set in the past so the first RefreshServiceStatus does not wait.
	LastSpatialServiceCheck = FDateTime::Now() - FTimespan::FromSeconds(RefreshFrequency);

	// Get the project name from the spatialos.json.
	ProjectName = GetProjectName();

	// Ensure the worker.jsons are up to date.
	WorkerBuildConfigAsync();

	// Watch the worker config directory for changes.
	StartUpWorkerConfigDirectoryWatcher();
}

FString FLocalDeploymentManager::GetSpotExe()
{
	FString PluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("UnrealGDK")));

	if (!FPaths::DirectoryExists(PluginDir))
	{
		// If the Project Plugin doesn't exist then use the Engine Plugin.
		PluginDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::EnginePluginsDir(), TEXT("UnrealGDK")));
		ensure(FPaths::DirectoryExists(PluginDir));
	}

	return  FPaths::ConvertRelativePathToFull(FPaths::Combine(PluginDir, TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/spot.exe")));
}

FString FLocalDeploymentManager::GetSpatialOSDirectory()
{
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/")));
}

void FLocalDeploymentManager::StartUpWorkerConfigDirectoryWatcher()
{
	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get();
	if (DirectoryWatcher)
	{
		// Watch the worker config directory for changes.
		const FString SpatialDirectory = GetSpatialOSDirectory();
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
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FString SpatialFileName = TEXT("spatialos.json");
	FString SpatialFileResult;
	FFileHelper::LoadFileToString(SpatialFileResult, *FPaths::Combine(SpatialDirectory, SpatialFileName));

	TSharedPtr<FJsonObject> JsonParsedSpatialFile;
	if (!ParseJson(SpatialFileResult, JsonParsedSpatialFile))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spatialos.json failed. Can't get project name."));
	}

	if (!JsonParsedSpatialFile->TryGetStringField(TEXT("name"), ProjectName))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'name' does not exist in spatialos.json. Can't read project name."));
	}

	return ProjectName;
}

void FLocalDeploymentManager::WorkerBuildConfigAsync()
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		FString SpatialExe = TEXT("spatial.exe");
		FString SpatialServiceStatusArgs = TEXT("worker build build-config");
		const FString SpatialDirectory = GetSpatialOSDirectory();

		FString WorkerBuildConfigResult;
		int32 ExitCode;
		ExecuteAndReadOutput(*SpatialExe, *SpatialServiceStatusArgs, *SpatialDirectory, WorkerBuildConfigResult, ExitCode);

		if (ExitCode == ExitCodeSuccess)
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Building worker configurations failed. Please ensure your .worker.json files are correct."));
		}
	});
}

bool FLocalDeploymentManager::ParseJson(FString RawJsonString, TSharedPtr<FJsonObject>& JsonParsed)
{
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(RawJsonString);
	return FJsonSerializer::Deserialize(JsonReader, JsonParsed);
}

void FLocalDeploymentManager::ExecuteAndReadOutput(FString Executable, FString Arguments, FString DirectoryToRun, FString& OutResult, int32& ExitCode)
{
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
		ProcHandle.Reset();
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Execution failed. '%s' with arguments '%s' in directory '%s' "), *Executable, *Arguments, *DirectoryToRun);
	}

	FPlatformProcess::ClosePipe(0, ReadPipe);
	FPlatformProcess::ClosePipe(0, WritePipe);
}

void FLocalDeploymentManager::RefreshServiceStatus()
{
	FDateTime CurrentTime = FDateTime::Now();

	FTimespan Span = CurrentTime - LastSpatialServiceCheck;

	if (Span.GetSeconds() < RefreshFrequency)
	{
		return;
	}

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		GetServiceStatus();
		GetLocalDeploymentStatus();
	});

	LastSpatialServiceCheck = FDateTime::Now();
}

bool FLocalDeploymentManager::TryStartLocalDeployment(FString LaunchConfig, FString LaunchArgs)
{
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

	FString SpotExe = GetSpotExe();
	FString SpotCreateArgs = FString::Printf(TEXT("alpha deployment create --launch-config=\"%s\" --name=localdeployment --project-name=%s --json %s"), *LaunchConfig, *ProjectName, *LaunchArgs);
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FDateTime SpotCreateStart = FDateTime::Now();

	FString SpotCreateResult;
	int32 ExitCode;
	ExecuteAndReadOutput(*SpotExe, *SpotCreateArgs, *SpatialDirectory, SpotCreateResult, ExitCode);
	bStartingDeployment = false;

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Creation of local deployment failed. %s"), *SpotCreateResult);
		return false;
	}

	bool bSuccess = false;

	TSharedPtr<FJsonObject> SpotJsonResult;
	bool bPasingSuccess = ParseJson(SpotCreateResult, SpotJsonResult);
	if (!bPasingSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot create result failed."));
	}

	const TSharedPtr<FJsonObject>* SpotJsonContent = nullptr;
	if (bPasingSuccess && !SpotJsonResult->TryGetObjectField(TEXT("content"), SpotJsonContent))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'content' does not exist in Json result from 'spot create': %s"), *SpotCreateResult);
		bPasingSuccess = false;
	}

	FString DeploymentStatus;
	if (bPasingSuccess && SpotJsonContent->Get()->TryGetStringField(TEXT("status"), DeploymentStatus))
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
	

	// Make sure the internal deployment state tracking is updated.
	IsLocalDeploymentRunning();
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

	FString SpotExe = GetSpotExe();
	FString SpotListArgs = FString::Printf(TEXT("alpha deployment delete --id=%s --json"), *LocalRunningDeploymentID);
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FString SpotDeleteResult;
	int32 ExitCode;
	ExecuteAndReadOutput(*SpotExe, *SpotListArgs, *SpatialDirectory, SpotDeleteResult, ExitCode);
	bStoppingDeployment = false;

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to stop local deployment! %s"), *SpotDeleteResult);
	}

	bool bSuccess = false;

	TSharedPtr<FJsonObject> SpotJsonResult;
	bool bPasingSuccess = ParseJson(SpotDeleteResult, SpotJsonResult);
	if (!bPasingSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot delete result failed."));
	}

	const TSharedPtr<FJsonObject>* SpotJsonContent = nullptr;
	if (bPasingSuccess && !SpotJsonResult->TryGetObjectField(TEXT("content"), SpotJsonContent))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("'content' does not exist in Json result from 'spot delete': %s"), *SpotDeleteResult);
		bPasingSuccess = false;
	}

	FString DeploymentStatus;
	if(bPasingSuccess && SpotJsonContent->Get()->TryGetStringField(TEXT("status"), DeploymentStatus))
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
	

	// Make sure the internal deployment state tracking is updated.
	IsLocalDeploymentRunning();
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

	FString SpatialExe = TEXT("spatial.exe");
	FString SpatialServiceStartArgs = TEXT("service start");
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FString ServiceStartResult;
	int32 ExitCode;
	ExecuteAndReadOutput(*SpatialExe, *SpatialServiceStartArgs, *SpatialDirectory, ServiceStartResult, ExitCode);
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
	if (!bSpatialServiceRunning)
	{
		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Tried to stop spatial service but it's not running."));
		return false;
	}

	bStoppingSpatialService = true;
	FString SpatialExe = TEXT("spatial.exe");
	FString SpatialServiceStartArgs = TEXT("service stop");
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FString ServiceStopResult;
	int32 ExitCode;
	ExecuteAndReadOutput(*SpatialExe, *SpatialServiceStartArgs, *SpatialDirectory, ServiceStopResult, ExitCode);
	bStoppingSpatialService = false;

	if (ExitCode == ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Spatial service stopped!"));
		bSpatialServiceRunning = false;
		bLocalDeploymentRunning = false;
		return true;
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Spatial service failed to stop! %s"), *ServiceStopResult);
	}

	IsSpatialServiceRunning();
	return false;
}

bool FLocalDeploymentManager::GetLocalDeploymentStatus()
{
	if (!bSpatialServiceRunning)
	{
		bLocalDeploymentRunning = false;
		return bLocalDeploymentRunning;
	}

	FString SpotExe = GetSpotExe();
	FString SpotListArgs = FString::Printf(TEXT("alpha deployment list --project-name=%s --json"), *ProjectName);
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FString SpotListResult;
	int32 ExitCode;
	ExecuteAndReadOutput(*SpotExe, *SpotListArgs, *SpatialDirectory, SpotListResult, ExitCode);

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to check local deployment status."));
		return false;
	}

	TSharedPtr<FJsonObject> SpotJsonResult;
	bool bPasingSuccess = ParseJson(SpotListResult, SpotJsonResult);
	if (!bPasingSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot list result failed."));
	}

	const TSharedPtr<FJsonObject>* SpotJsonContent = nullptr;
	if (bPasingSuccess && SpotJsonResult->TryGetObjectField(TEXT("content"), SpotJsonContent))
	{
		const TArray<TSharedPtr<FJsonValue>>* JsonDeployments;
		if(!SpotJsonContent->Get()->TryGetArrayField(TEXT("deployments"), JsonDeployments))
		{
			UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("No local deployments running."));
			return false;
		}

		for (TSharedPtr<FJsonValue> JsonDeployment : *JsonDeployments)
		{
			FString DeploymentStatus;
			if(JsonDeployment->AsObject()->TryGetStringField(TEXT("status"), DeploymentStatus))
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
	FString SpatialExe = TEXT("spatial.exe");
	FString SpatialServiceStatusArgs = TEXT("service status");
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FString ServiceStatusResult;
	int32 ExitCode;
	ExecuteAndReadOutput(*SpatialExe, *SpatialServiceStatusArgs, *SpatialDirectory, ServiceStatusResult, ExitCode);

	if (ExitCode != ExitCodeSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to check spatial service status: %s"), *ServiceStatusResult);
	}

	if (ServiceStatusResult.Contains(TEXT("Local API service is not running.")))
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Spatial service not running."));
		bSpatialServiceRunning = false;
		bLocalDeploymentRunning = false;
		return false;
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Spatial service running."));
		bSpatialServiceRunning = true;
		return true;
	}

	return false;
}

bool FLocalDeploymentManager::IsLocalDeploymentRunning()
{
	return bLocalDeploymentRunning;
}

bool FLocalDeploymentManager::IsSpatialServiceRunning()
{
	return bSpatialServiceRunning;
}

bool FLocalDeploymentManager::IsDeploymentStarting()
{
	return bStartingDeployment;
}

bool FLocalDeploymentManager::IsDeploymentStopping()
{
	return bStoppingDeployment;
}

bool FLocalDeploymentManager::IsServiceStarting()
{
	return bStartingSpatialService;
}

bool FLocalDeploymentManager::IsServiceStopping()
{
	return bStoppingSpatialService;
}
