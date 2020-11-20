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
	TArray<int32> RequiredRuntimePorts = { RequiredRuntimePort, WorkerPort, HTTPPort, GRPCPort };

	for (int32 RuntimePort : RequiredRuntimePorts)
	{
		if (CheckIfPortIsBound(RequiredRuntimePort))
		{
			// If it exists offer the user the ability to kill it.
			if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("KillPortBlockingProcess",
																 "A required port is blocked by another process (potentially by an old "
																 "deployment). Would you like to kill this process?"))
				== EAppReturnType::Yes)
			{
				bSuccess = bSuccess && KillProcessBlockingPort(RequiredRuntimePort);
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
	FDateTime StartTime = FDateTime::Now();

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

	// TODO: Use this as an additional input arg when we can use 15.0.0 (when we have ComponentSets)
	FString SchemaBundle = SpatialGDKServicesConstants::SchemaBundlePath;
	FString SnapshotPath = SpatialGDKServicesConstants::SpatialOSSnapshotFolderPath;

	// runtime.exe --config=squid_config.json --snapshot=snapshots\default.snapshot --worker-port 8018 --http-port 5006 --grpc-port 7777
	// --worker-external-host 127.0.0.1 --snapshots-directory=spatial/snapshots.
	FString RuntimeArgs = FString::Printf(TEXT("--config=\"%s\" --snapshot=\"%s\" --worker-port %s --http-port %s --grpc-port %s "
											   "--snapshots-directory=\"%s\" %s"),
										  *LaunchConfig, *SnapshotName, *FString::FromInt(WorkerPort), *FString::FromInt(HTTPPort),
										  *FString::FromInt(GRPCPort), *SnapshotPath, *LaunchArgs);

	if (!RuntimeIPToExpose.IsEmpty())
	{
		RuntimeArgs.Append(FString::Printf(TEXT(" --worker-external-host %s"), *RuntimeIPToExpose));
	}

	FString RuntimePath = SpatialGDKServicesConstants::GetRuntimeExecutablePath(RuntimeVersion);

	RuntimeProcess = { *RuntimePath, *RuntimeArgs, SpatialGDKServicesConstants::SpatialOSDirectory, /*InHidden*/ true,
					   /*InCreatePipes*/ true };

	FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
	TWeakPtr<SSpatialOutputLog> SpatialOutputLog = GDKServices.GetSpatialOutputLog();

	RuntimeProcess->OnOutput().BindLambda([&, SpatialOutputLog](const FString& Output) {
		SpatialOutputLog.Pin()->FormatAndPrintRawLogLine(Output);

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

	FTimespan Span = FDateTime::Now() - StartTime;
	UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Successfully created local deployment in %f seconds."), Span.GetTotalSeconds());

	AsyncTask(ENamedThreads::GameThread, [this] {
		OnDeploymentStart.Broadcast();
	});

	return;
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

// TODO: UNR-4423 Change this to taking a snapshot without the services
void SPATIALGDKSERVICES_API FLocalDeploymentManager::TakeSnapshot(UWorld* World, FSpatialSnapshotTakenFunc OnSnapshotTaken)
{
	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
	TSharedRef<IHttpRequest> HttpRequest = HttpModule.Get().CreateRequest();

	HttpRequest->OnProcessRequestComplete().BindLambda([World, OnSnapshotTaken](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse,
																				bool bSucceeded) {
		if (!bSucceeded)
		{
			UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to trigger snapshot at '%s'; received '%s'"), *HttpRequest->GetURL(),
				   *HttpResponse->GetContentAsString());
			if (OnSnapshotTaken != nullptr)
			{
				OnSnapshotTaken(false /* bSuccess */, FString() /* PathToSnapshot */);
			}
			return;
		}

		// Unfortunately by the time this callback happens, the files haven't been flushed, so if you copy you may get
		// the wrong info! So let's wait a bit..
		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(
			TimerHandle,
			[OnSnapshotTaken, HttpResponse]() {
				bool bSuccess = false;

				FString SnapshotSearchDirectory = SpatialGDKServicesConstants::SpatialOSSnapshotFolderPath;

				FSpatialGDKServicesModule& GDKServices = FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
				FLocalDeploymentManager* LocalDeploymentManager = GDKServices.GetLocalDeploymentManager();

				IFileManager& FileManager = FFileManagerGeneric::Get();

				FString NewestSnapshotFilePath = HttpResponse->GetContentAsString();
				FPaths::NormalizeFilename(NewestSnapshotFilePath);

				bSuccess = FPaths::FileExists(NewestSnapshotFilePath);

				if (!bSuccess)
				{
					UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed find snapshot file in '%s'"), *SnapshotSearchDirectory);
				}

				if (OnSnapshotTaken != nullptr)
				{
					OnSnapshotTaken(bSuccess, NewestSnapshotFilePath);
				}
			},
			0.5f /* InRate */, false /* InbLoop */);
	});

	HttpRequest->SetURL(TEXT("http://localhost:5006/snapshot"));
	HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
	HttpRequest->SetVerb("GET");

	HttpRequest->ProcessRequest();
}

#undef LOCTEXT_NAMESPACE
