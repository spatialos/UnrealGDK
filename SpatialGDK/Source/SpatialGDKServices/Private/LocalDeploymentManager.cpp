// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalDeploymentManager.h"

#include "Async/Async.h"

//#include "SpatialGDKEditor.h"
//#include "SpatialGDKSettings.h"
//#include "SpatialGDKEditorSettings.h"
//#include "SpatialGDKEditorSettings.h"

#include "Interop/Connection/EditorWorkerController.h"

#include "HAL/FileManager.h"

#include "AssetRegistryModule.h"
#include "GeneralProjectSettings.h"
#include "Misc/FileHelper.h"

#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/PlatformFilemanager.h"
#include "FileCache.h"
#include "DirectoryWatcherModule.h"

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
	LastSpatialServiceCheck = FDateTime::Now() - FTimespan::FromSeconds(2);

	// Get the project name from the spatialos.json.
	ProjectName = GetProjectName();

	// Ensure the worker.jsons are up to date.
	WorkerBuildConfigAsync();

	// Watch the worker config directory for changes.
	StartUpDirectoryWatcher();
}

void FLocalDeploymentManager::Tick(float DeltaTime)
{
	RefreshServiceStatus();
}

FString FLocalDeploymentManager::GetSpotPath()
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

void FLocalDeploymentManager::StartUpDirectoryWatcher()
{
	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get();
	if (DirectoryWatcher)
	{
		// Watch the worker config directory for changes.
		const FString SpatialDirectory = GetSpatialOSDirectory();
		// TODO: If directory exists otherwise panic.
		FString WorkerConfigDirectory = FPaths::Combine(SpatialDirectory, TEXT("workers"));
		WorkerConfigDirectoryChangedDelegate = IDirectoryWatcher::FDirectoryChanged::CreateRaw(this, &FLocalDeploymentManager::OnWorkerConfigDirectoryChanged);
		DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(WorkerConfigDirectory, WorkerConfigDirectoryChangedDelegate, WorkerConfigDirectoryChangedDelegateHandle);
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

	TSharedPtr<FJsonObject> JsonParsedSpatialFile = ParseJson(SpatialFileResult);

	if (!JsonParsedSpatialFile.IsValid())
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
	FString SpatialExe = TEXT("spatial.exe");
	FString SpatialServiceStatusArgs = TEXT("worker build build-config");
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FString WorkerBuildConfigResult;
	ExecuteAndReadOutput(*SpatialExe, *SpatialServiceStatusArgs, *SpatialDirectory, WorkerBuildConfigResult);

	if (!WorkerBuildConfigResult.Contains(TEXT("succeeded")))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Building worker configurations failed. Please ensure your .worker.json files are correct."));
	}
}

TSharedPtr<FJsonObject> FLocalDeploymentManager::ParseJson(FString RawJsonString)
{
	TSharedPtr<FJsonObject> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(RawJsonString);
	FJsonSerializer::Deserialize(JsonReader, JsonParsed);

	return JsonParsed;
}

void FLocalDeploymentManager::ExecuteAndReadOutput(FString Executable, FString Arguments, FString DirectoryToRun, FString& OutResult)
{
	UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Attempting to execute '%s' with arguments '%s' in directory '%s'"), *Executable, *Arguments, *DirectoryToRun);

	// Create pipes to read output of spot.
	void* ReadPipe;
	void* WritePipe;
	if (!FPlatformProcess::CreatePipe(ReadPipe, WritePipe))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Command execution failed. Could not create read and write pipes."));
	}

	FProcHandle ProcHandle;
	uint32 ProcessID;
	FString WritePipeResult;

	ProcHandle = FPlatformProcess::CreateProc(*Executable, *Arguments, false, true, true, &ProcessID, 1 /*PriorityModifer*/, *DirectoryToRun, WritePipe, ReadPipe);

	if (!ProcHandle.IsValid())
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Command execution failed. '%s' with arguments '%s' in directory '%s' "), *Executable, *Arguments, *DirectoryToRun);
		return;
	}

	WritePipeResult = FPlatformProcess::ReadPipe(WritePipe);
	OutResult = FPlatformProcess::ReadPipe(ReadPipe);

	// Make sure the pipe doesn't get clogged.
	AsyncTask(ENamedThreads::AnyBackgroundHiPriTask, [&ReadPipe, &OutResult, &ProcHandle]
	{
		while (FPlatformProcess::IsProcRunning(ProcHandle))
		{
			// TODO: Reading from this pipe when debugging causes a crash.
			if (ReadPipe)
			{
				OutResult += FPlatformProcess::ReadPipe(ReadPipe);
			}
		}
	});

	if (FPlatformProcess::IsProcRunning(ProcHandle))
	{
		FPlatformProcess::WaitForProc(ProcHandle);
	}
}

void FLocalDeploymentManager::RefreshServiceStatus()
{
	FDateTime CurrentTime = FDateTime::Now();

	FTimespan Span = CurrentTime - LastSpatialServiceCheck;

	if (Span.GetSeconds() < 3)
	{
		return;
	}

	// TODO: Auto start spatial service based on config value
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

	// If the service is not running then start it.
	if (!bSpatialServiceRunning)
	{
		TryStartSpatialService();
	}

	bStartingDeployment = true;

	FString SpotExe = GetSpotPath();
	FString SpotCreateArgs = FString::Printf(TEXT("alpha deployment create --launch-config=\"%s\" --name=testname --project-name=%s --json %s"), *LaunchConfig, *ProjectName, *LaunchArgs);
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FDateTime SpotCreateStart = FDateTime::Now();

	FString SpotCreateResult;
	ExecuteAndReadOutput(*SpotExe, *SpotCreateArgs, *SpatialDirectory, SpotCreateResult);
	bStartingDeployment = false;

	// TODO: ParseJson should return a bool to check for failures.
	TSharedPtr<FJsonObject> SpotJsonResult = ParseJson(SpotCreateResult);
	if (!SpotJsonResult.IsValid())
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot create result failed."));
		LocalRunningDeploymentID.Empty();
		bLocalDeploymentRunning = false;
	}

	TSharedPtr<FJsonObject> SpotJsonContent = SpotJsonResult->GetObjectField(TEXT("content"));

	FString DeploymentStatus = SpotJsonContent->GetStringField(TEXT("status"));
	if (DeploymentStatus == TEXT("RUNNING"))
	{
		FString DeploymentID = SpotJsonContent->GetStringField(TEXT("id"));
		LocalRunningDeploymentID = DeploymentID;
		bLocalDeploymentRunning = true;

		FDateTime SpotCreateEnd = FDateTime::Now();
		FTimespan Span = SpotCreateEnd - SpotCreateStart;

		OnDeploymentStart.Broadcast();

		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Successfully created local deployment in %f seconds."), Span.GetTotalSeconds());
		// TODO: refactor into bSuccess
		IsLocalDeploymentRunning();
		return true;
	}
	else
	{
		bLocalDeploymentRunning = false;

		// TODO: Is this the correct log path anymore?
		const FString SpatialLogPath = GetSpatialOSDirectory() + FString(TEXT("/logs/spatial.log"));
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to start SpatialOS, please refer to log file `%s` for more information."), *SpatialLogPath);
		IsLocalDeploymentRunning();
		return false;
	}
}

bool FLocalDeploymentManager::TryStopLocalDeployment()
{
	// if (bSpatialServiceRunning && bLocalDeploymentRunning) 
	if (!bLocalDeploymentRunning || LocalRunningDeploymentID.IsEmpty())
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Tried to stop local deployment but no active deployment exists."));
		return false;
	}

	bStoppingDeployment = true;

	FString SpotExe = GetSpotPath();
	FString SpotListArgs = FString::Printf(TEXT("alpha deployment delete --id=%s --json"), *LocalRunningDeploymentID);
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FString SpotDeleteResult;
	ExecuteAndReadOutput(*SpotExe, *SpotListArgs, *SpatialDirectory, SpotDeleteResult);
	bStoppingDeployment = false;

	bool bSuccess = false;

	if (SpotDeleteResult.Contains(TEXT("failed to get deployment with id")))
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to stop local deployment! %s"), *SpotDeleteResult);
		IsLocalDeploymentRunning();
		return bSuccess;
	}

	TSharedPtr<FJsonObject> SpotJsonResult = ParseJson(SpotDeleteResult);
	if (!SpotJsonResult.IsValid())
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot delete result failed."));
		return bSuccess;
	}

	TSharedPtr<FJsonObject> SpotJsonContent = SpotJsonResult->GetObjectField(TEXT("content"));

	FString DeploymentStatus = SpotJsonContent->GetStringField(TEXT("status"));
	if (DeploymentStatus == TEXT("STOPPED"))
	{
		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Successfully stopped local deplyoment"));
		LocalRunningDeploymentID.Empty();
		bLocalDeploymentRunning = false;
		bSuccess = true;
	}
	else
	{
		// TODO: Wtf happened here.
	}

	// Make sure deployment state internally is updated.
	IsLocalDeploymentRunning();
	return bSuccess;
}

bool FLocalDeploymentManager::TryStartSpatialService()
{
	// TODO: Wrap this in if running code
	bStartingSpatialService = true;

	FString SpatialExe = TEXT("spatial.exe");
	FString SpatialServiceStartArgs = TEXT("service start");
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FString ServiceStartResult;
	ExecuteAndReadOutput(*SpatialExe, *SpatialServiceStartArgs, *SpatialDirectory, ServiceStartResult);
	bStartingSpatialService = false;

	// TODO: More robust result check
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
	// TODO: Wrap this in if service running. 
	bStoppingSpatialService = true;

	FString SpatialExe = TEXT("spatial.exe");
	FString SpatialServiceStartArgs = TEXT("service stop");
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FString ServiceStartResult;
	ExecuteAndReadOutput(*SpatialExe, *SpatialServiceStartArgs, *SpatialDirectory, ServiceStartResult);
	bStoppingSpatialService = false;

	// TODO: More robust result check
	if (ServiceStartResult.Contains(TEXT("Local API service stopped")))
	{
		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Spatial service stopped!"));
		bSpatialServiceRunning = false;
		bLocalDeploymentRunning = false;
		return true;
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Spatial service failed to stop! %s"), *ServiceStartResult);

		// Who knows what happened. Do a service status.
		IsSpatialServiceRunning();
		return false;
	}
}

bool FLocalDeploymentManager::GetLocalDeploymentStatus()
{
	if (!bSpatialServiceRunning)
	{
		bLocalDeploymentRunning = false;
		return bLocalDeploymentRunning;
	}

	FString SpotExe = GetSpotPath();

	FString SpotListArgs = FString::Printf(TEXT("alpha deployment list --project-name=%s --json"), *ProjectName);
	const FString SpatialDirectory = GetSpatialOSDirectory();

	FString SpotListResult;
	ExecuteAndReadOutput(*SpotExe, *SpotListArgs, *SpatialDirectory, SpotListResult);

	if (SpotListResult.IsEmpty())
	{
		return false;
	}

	TSharedPtr<FJsonObject> SpotJsonResult = ParseJson(SpotListResult);
	if (!SpotJsonResult.IsValid())
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot result failed. Can't check deployment status."));
	}

	bool bParsingSuccess;

	const TSharedPtr<FJsonObject>* SpotJsonContent;
	bParsingSuccess = SpotJsonResult->TryGetObjectField(TEXT("content"), SpotJsonContent);

	if (!bParsingSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Json parsing of spot result failed. Can't check deployment status."));
		return false;
	}

	// TODO: This can be null if no deployments exist.
	const TArray<TSharedPtr<FJsonValue>>* JsonDeployments;
	bParsingSuccess = SpotJsonContent->Get()->TryGetArrayField(TEXT("deployments"), JsonDeployments);

	if (!bParsingSuccess)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("No local deployments running."));
		return false;
	}

	for (TSharedPtr<FJsonValue> JsonDeployment : *JsonDeployments)
	{
		FString DeploymentStatus = JsonDeployment->AsObject()->GetStringField(TEXT("status"));
		if (DeploymentStatus == TEXT("RUNNING"))
		{
			FString DeploymentId = JsonDeployment->AsObject()->GetStringField(TEXT("id"));

			UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Running deployment found: %s"), *DeploymentId);

			// TODO: Better way of getting current running deployment ID.
			LocalRunningDeploymentID = DeploymentId;
			bLocalDeploymentRunning = true;

			return true;
		}
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
	ExecuteAndReadOutput(*SpatialExe, *SpatialServiceStatusArgs, *SpatialDirectory, ServiceStatusResult);

	// TODO: More robust result check
	if (ServiceStatusResult.Contains(TEXT("Local API service is not running")))
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
