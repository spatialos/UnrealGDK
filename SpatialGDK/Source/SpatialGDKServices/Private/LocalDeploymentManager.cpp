// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LocalDeploymentManager.h"

#include "AssetRegistryModule.h"
#include "Async/Async.h"
#include "DirectoryWatcherModule.h"
#include "Editor.h"
#include "Engine/World.h"
#include "FileCache.h"
#include "GeneralProjectSettings.h"
#include "HAL/FileManagerGeneric.h"
#include "HttpModule.h"
#include "IPAddress.h"
#include "Improbable/SpatialGDKSettingsBridge.h"
#include "Interfaces/IHttpResponse.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Regex.h"
#include "Json/Public/Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/MonitoredProcess.h"
#include "SSpatialOutputLog.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "SpatialCommandUtils.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "UObject/CoreNet.h"

DEFINE_LOG_CATEGORY(LogSpatialDeploymentManager);

#define LOCTEXT_NAMESPACE "FLocalDeploymentManager"

FLocalDeploymentManager::FLocalDeploymentManager()
	: bLocalDeploymentRunning(false)
	, bStartingDeployment(false)
	, bStoppingDeployment(false)
{
}

void FLocalDeploymentManager::PreInit(bool bChinaEnabled)
{
	// Ensure the worker.jsons are up to date.
	WorkerBuildConfigAsync();

	// Watch the worker config directory for changes.
	StartUpWorkerConfigDirectoryWatcher();
}

void FLocalDeploymentManager::Init()
{
	// Kill any existing runtime processes.
	// We cannot attach to old runtime processes as they may be 'zombie' and not killable (even if they are not blocking ports).
	// Usually caused by a driver bug, see: https://stackoverflow.com/questions/49988/really-killing-a-process-in-windows
	SpatialCommandUtils::TryKillProcessWithName(SpatialGDKServicesConstants::RuntimeExe);
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
	const bool ShouldRebuild = FileChanges.ContainsByPredicate([](FFileChangeData& FileChange) {
		return FileChange.Filename.EndsWith(".worker.json");
	});

	if (ShouldRebuild)
	{
		UE_LOG(LogSpatialDeploymentManager, Log,
			   TEXT("Worker config files updated. Regenerating worker descriptors ('spatial worker build build-config')."));

		WorkerBuildConfigAsync();
	}
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

	// Check for the known runtime ports which could be blocked by other processes.
	TArray<int32> RequiredRuntimePorts = { RequiredRuntimePort, WorkerPort, HTTPPort, SpatialGDKServicesConstants::RuntimeGRPCPort };

	for (int32 RuntimePort : RequiredRuntimePorts)
	{
		if (CheckIfPortIsBound(RequiredRuntimePort))
		{
			// If it exists offer the user the ability to kill it.
			FText DialogMessage = LOCTEXT("KillPortBlockingProcess",
										  "A required port is blocked by another process (potentially by an old "
										  "deployment). Would you like to kill this process?");
			if (FMessageDialog::Open(EAppMsgType::YesNo, DialogMessage) == EAppReturnType::Yes)
			{
				bSuccess &= KillProcessBlockingPort(RequiredRuntimePort);
			}
			else
			{
				bSuccess = false;
			}
		}
	}

	return bSuccess;
}

void FLocalDeploymentManager::TryStartLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs,
													  FString SnapshotName, FString RuntimeIPToExpose,
													  const LocalDeploymentCallback& CallBack)
{
	RuntimeStartTime = FDateTime::Now();

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

	bStartingDeployment = true;

	FString SchemaBundle = SpatialGDKServicesConstants::SchemaBundlePath;

	// Give the snapshot path a timestamp to ensure we don't overwrite snapshots from older deployments.
	// The snapshot service saves snapshots with the name `snapshot-n.snapshot` for a given deployment,
	// where 'n' is the number of snapshots taken since starting the deployment.
	FString SnapshotPath = FPaths::Combine(SpatialGDKServicesConstants::SpatialOSSnapshotFolderPath, *RuntimeStartTime.ToString());

	// Create the folder for storing the snapshots.
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.CreateDirectoryTree(*SnapshotPath);

	// Use the runtime start timestamp as the log directory, e.g. `<Project>/spatial/localdeployment/<timestamp>/`
	FString LocalDeploymentLogsDir = FPaths::Combine(SpatialGDKServicesConstants::LocalDeploymentLogsDir, RuntimeStartTime.ToString());

	// runtime.exe --config=squid_config.json --snapshot=snapshots/default.snapshot --worker-port 8018 --http-port 5006 --grpc-port 7777
	// --worker-external-host 127.0.0.1 --snapshots-directory=spatial/snapshots/<timestamp>
	// --schema-bundle=spatial/build/assembly/schema/schema.sb
	// --event-tracing-logs-directory=`<Project>/spatial/localdeployment/<timestamp>/`
	FString RuntimeArgs =
		FString::Printf(TEXT("--config=\"%s\" --snapshot=\"%s\" --worker-port %s --http-port=%s --grpc-port=%s "
							 "--snapshots-directory=\"%s\" --schema-bundle=\"%s\" --event-tracing-logs-directory=\"%s\" %s"),
						*LaunchConfig, *SnapshotName, *FString::FromInt(WorkerPort), *FString::FromInt(HTTPPort),
						*FString::FromInt(SpatialGDKServicesConstants::RuntimeGRPCPort), *SnapshotPath, *SchemaBundle,
						*LocalDeploymentLogsDir, *LaunchArgs);

	if (!RuntimeIPToExpose.IsEmpty())
	{
		RuntimeArgs.Append(FString::Printf(TEXT(" --worker-external-host %s"), *RuntimeIPToExpose));
	}

	// Setup the runtime file logger.
	SetupRuntimeFileLogger(LocalDeploymentLogsDir);

	FString RuntimePath = SpatialGDKServicesConstants::GetRuntimeExecutablePath(RuntimeVersion);

	RuntimeProcess = { *RuntimePath, *RuntimeArgs, SpatialGDKServicesConstants::SpatialOSDirectory, /*InHidden*/ true,
					   /*InCreatePipes*/ true };

	FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
	TWeakPtr<SSpatialOutputLog> SpatialOutputLog = GDKServices.GetSpatialOutputLog();

	RuntimeProcess->OnOutput().BindLambda([&RuntimeLogFileHandle = RuntimeLogFileHandle, &bStartingDeployment = bStartingDeployment,
										   SpatialOutputLog](const FString& Output) {
		// Format and output the log to the editor window `SpatialOutputLog`
		SpatialOutputLog.Pin()->FormatAndPrintRawLogLine(Output);

		// Save the raw runtime output to disk.
		if (RuntimeLogFileHandle.IsValid())
		{
			// In order to get the correct length of the ANSI converted string, we must create the converted string here.
			auto OutputANSI = StringCast<ANSICHAR>(*Output);

			RuntimeLogFileHandle->Write((const uint8*)OutputANSI.Get(), OutputANSI.Length());

			// Always add a newline
			RuntimeLogFileHandle->Write((const uint8*)LINE_TERMINATOR_ANSI, 1);
		}

		// Timeout detection.
		if (bStartingDeployment && Output.Contains(TEXT("startup completed")))
		{
			bStartingDeployment = false;
		}
	});

	RuntimeProcess->Launch();

	while (bStartingDeployment && RuntimeProcess->Update())
	{
		if (RuntimeProcess->GetDuration().GetTotalSeconds() > RuntimeTimeout)
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Timed out waiting for the Runtime to start."));
			bStartingDeployment = false;
			break;
		}
	}

	bStartingDeployment = false;
	bLocalDeploymentRunning = true;

	FTimespan Span = FDateTime::Now() - RuntimeStartTime;
	UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Successfully created local deployment in %f seconds."), Span.GetTotalSeconds());

	AsyncTask(ENamedThreads::GameThread, [this] {
		OnDeploymentStart.Broadcast();
	});

	return;
}

bool FLocalDeploymentManager::SetupRuntimeFileLogger(const FString& RuntimeLogDir)
{
	// Ensure any old log file is cleaned up.
	RuntimeLogFileHandle.Reset();

	FString RuntimeLogFilePath = FPaths::Combine(RuntimeLogDir, TEXT("runtime.log"));
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	const bool bSuccess = PlatformFile.CreateDirectoryTree(*RuntimeLogDir);

	if (bSuccess)
	{
		RuntimeLogFileHandle.Reset(PlatformFile.OpenWrite(*RuntimeLogFilePath, /*bAppend*/ false, /*bAllowRead*/ true));
	}

	if (!bSuccess || RuntimeLogFileHandle == nullptr)
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Could not create runtime log file at '%s'. Saving logs to disk will be disabled."),
			   *RuntimeLogFilePath);
		return false;
	}

	UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Runtime logs will be saved to %s"), *RuntimeLogFilePath);
	return true;
}

bool FLocalDeploymentManager::TryStopLocalDeployment()
{
	if (!bLocalDeploymentRunning)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Tried to stop local deployment but no active deployment exists."));
		return false;
	}

	bStoppingDeployment = true;
	RuntimeProcess->Stop();

	double RuntimeStopTime = RuntimeProcess->GetDuration().GetTotalSeconds();

	// Update returns true while the process is still running. Wait for it to finish.
	while (RuntimeProcess->Update())
	{
		// If the runtime did not stop after some timeout then inform the user as something is amiss.
		if (RuntimeProcess->GetDuration().GetTotalSeconds() > RuntimeStopTime + RuntimeTimeout)
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Timed out waiting for the Runtime to stop."));
			bStoppingDeployment = false;
			return false;
		}
	}

	// Kill the log file handle.
	RuntimeLogFileHandle.Reset();

	bLocalDeploymentRunning = false;
	bStoppingDeployment = false;

	return true;
}

bool FLocalDeploymentManager::IsLocalDeploymentRunning() const
{
	return bLocalDeploymentRunning;
}

bool FLocalDeploymentManager::IsDeploymentStarting() const
{
	return bStartingDeployment;
}

bool FLocalDeploymentManager::IsDeploymentStopping() const
{
	return bStoppingDeployment;
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

void SPATIALGDKSERVICES_API FLocalDeploymentManager::TakeSnapshot(UWorld* World, FSpatialSnapshotTakenFunc OnSnapshotTaken)
{
	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
	TSharedRef<IHttpRequest> HttpRequest = HttpModule.Get().CreateRequest();

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[World, OnSnapshotTaken](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded) {
			if (!bSucceeded)
			{
				UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to trigger snapshot at '%s'; received '%s'"),
					   *HttpRequest->GetURL(), *HttpResponse->GetContentAsString());
				if (OnSnapshotTaken != nullptr)
				{
					OnSnapshotTaken(false /* bSuccess */, FString() /* PathToSnapshot */);
				}
				return;
			}

			FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
			FLocalDeploymentManager* LocalDeploymentManager = GDKServices.GetLocalDeploymentManager();

			IFileManager& FileManager = FFileManagerGeneric::Get();

			FString NewestSnapshotFilePath = HttpResponse->GetContentAsString();
			FPaths::NormalizeFilename(NewestSnapshotFilePath);

			bool bSuccess = FPaths::FileExists(NewestSnapshotFilePath);

			if (!bSuccess)
			{
				UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed find snapshot file at '%s'"), *NewestSnapshotFilePath);
			}

			if (OnSnapshotTaken != nullptr)
			{
				OnSnapshotTaken(bSuccess, NewestSnapshotFilePath);
			}
		});

	HttpRequest->SetURL(TEXT("http://localhost:5006/snapshot"));
	HttpRequest->SetVerb("GET");

	HttpRequest->ProcessRequest();
}

#undef LOCTEXT_NAMESPACE
