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
	// Ensure the worker.jsons are up to date.
	// TOOD: Probably remove this
	WorkerBuildConfigAsync();

	// Watch the worker config directory for changes.
	StartUpWorkerConfigDirectoryWatcher();
}

void FLocalDeploymentManager::Init(FString RuntimeIPToExpose)
{
	// Check if a runtime process is already running. This can either be an old editor started runtime or a runtime started outside the
	// editor.
	if (FPlatformProcess::IsApplicationRunning(TEXT("runtime.exe")))
	{
		bExistingRuntimeStarted = true;
	}
}

// TOOD: Do we need this with local standalone or just for the classic platform deployments?
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

// TOOD: Do we need this with local standalone or just for the classic platform deployments?
void FLocalDeploymentManager::OnWorkerConfigDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
{
	UE_LOG(LogSpatialDeploymentManager, Log,
		   TEXT("Worker config files updated. Regenerating worker descriptors ('spatial worker build build-config')."));
	WorkerBuildConfigAsync();
}

// TOOD: Do we need this with local standalone or just for the classic platform deployments?
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

void FLocalDeploymentManager::KillExistingRuntime()
{
	FPlatformProcess::FProcEnumerator ProcessIt;
	do
	{
		if (ProcessIt.GetCurrent().GetName().Equals(TEXT("runtime.exe")))
		{
			UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Killing runtime process: %d"), ProcessIt.GetCurrent().GetPID());
			auto Handle = FPlatformProcess::OpenProcess(ProcessIt.GetCurrent().GetPID());
			FPlatformProcess::TerminateProc(Handle);
		}
	} while (ProcessIt.MoveNext());
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

bool FLocalDeploymentManager::UseExistingRuntime()
{
	if (bExistingRuntimeStarted)
	{
		bExistingRuntimeStarted = false;

		// If an existing runtime still exists, ask if the user wants to restart it or carry on using it.
		if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("KillExistingRuntime",
															 "An existing local deployment runtime is still running."
															 "Would you like to restart the local deployment?"))
			== EAppReturnType::Yes)
		{
			// If yes kill the old runtime and move forward.
			KillExistingRuntime();
		}
		else
		{
			// If no, get a handle for the old runtime and use it for the current local deployment.
			FPlatformProcess::FProcEnumerator ProcessIt;
			do
			{
				if (ProcessIt.GetCurrent().GetName().Equals(TEXT("runtime.exe")))
				{
					RuntimeProcID = ProcessIt.GetCurrent().GetPID();
					RuntimeProc = FPlatformProcess::OpenProcess(RuntimeProcID);
				}
			} while (ProcessIt.MoveNext());

			return true;
		}
	}

	return false;
}

// TODO: Change this to starting the runtime binary
bool FLocalDeploymentManager::FinishLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs, FString SnapshotName,
													FString RuntimeIPToExpose)
{
	// runtime.exe --config=squid_config.json --snapshot=snapshots\default.snapshot --worker-port 8018 --http-port 5006 --grpc-port 7777

	LaunchConfig = TEXT("B:\\UnrealEngine425\\Samples\\UnrealGDKExampleProject\\spatial\\squid_config.json");
	FString SnapshotPath = TEXT("B:\\UnrealEngine425\\Samples\\UnrealGDKExampleProject\\spatial\\snapshots\\default.snapshot");

	FString RuntimeArgs = FString::Printf(TEXT("--config=\"%s\" --snapshot=\"%s\" --worker-port %s --http-port %s --grpc-port %s %s"),
										  *LaunchConfig, *SnapshotPath, *FString::FromInt(WorkerPort), *FString::FromInt(HTTPPort),
										  *FString::FromInt(GRPCPort), *LaunchArgs);

	FDateTime StartTime = FDateTime::Now();

	FString RuntimePath = TEXT("B:\\UnrealEngine425\\Samples\\UnrealGDKExampleProject\\spatial\\runtime.exe");

	uint32 OutProcessID;

	FPlatformProcess::CreatePipe(ReadPipe, WritePipe);

	// const TCHAR* URL, const TCHAR* Parms, bool bLaunchDetached, bool bLaunchHidden, bool bLaunchReallyHidden, uint32* OutProcessID, int32
	// PriorityModifier, const TCHAR* OptionalWorkingDirectory, void* PipeWriteChild, void * PipeReadChild)
	// TODO: Ignore the pipes for now but implement later.
	RuntimeProc = FPlatformProcess::CreateProc(*RuntimePath, *RuntimeArgs, false, true, true, &OutProcessID, 0, nullptr, WritePipe);

	RuntimeProcID = OutProcessID;

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this] {
		while (FPlatformProcess::IsProcRunning(RuntimeProc))
		{
			FString RuntimeLogs = FPlatformProcess::ReadPipe(ReadPipe);

			// Split the logs into newlines
			TArray<FString> RuntimeLogLines;
			RuntimeLogs.ParseIntoArrayLines(RuntimeLogLines);

			for (FString LogLine : RuntimeLogLines)
			{
				// TODO: We might want to log these to a separate place or to disk.
				// TODO: This will need proper categories
				UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Runtime: %s"), *LogLine);
			}

			FPlatformProcess::Sleep(0.01f);
		}
	});

	if (RuntimeProc.IsValid())
	{
		UE_LOG(LogSpatialDeploymentManager, Warning, TEXT("RuntimeProc is valid"));
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("RuntimeProc is not valid"));
	}

	if (FPlatformProcess::IsProcRunning(RuntimeProc))
	{
		UE_LOG(LogSpatialDeploymentManager, Warning, TEXT("RuntimeProc is running"));
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("RuntimeProc is not running"));
	}

	bStartingDeployment = false;
	bool bSuccess = false;
	bLocalDeploymentRunning = true;

	AsyncTask(ENamedThreads::GameThread, [this] {
		OnDeploymentStart.Broadcast();
	});

	return true;
}

// TODO: Might not need all these additional checks now
void FLocalDeploymentManager::TryStartLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs,
													  FString SnapshotName, FString RuntimeIPToExpose,
													  const LocalDeploymentCallback& CallBack)
{
	// TODO: Can probs get rid of this now.
	if (bLocalDeploymentRunning)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Tried to start a local deployment but one is already running."));
		if (CallBack)
		{
			CallBack(false);
		}
		return;
	}

	if (UseExistingRuntime())
	{
		UE_LOG(LogSpatialDeploymentManager, Log, TEXT("Using existing runtime that was started before the editor was booted."));
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

	FinishLocalDeployment(LaunchConfig, RuntimeVersion, LaunchArgs, SnapshotName, RuntimeIPToExpose);

	return;
}

// TODO: Change this to killing the runtime binary
bool FLocalDeploymentManager::TryStopLocalDeployment()
{
	if (!bLocalDeploymentRunning)
	{
		UE_LOG(LogSpatialDeploymentManager, Verbose, TEXT("Tried to stop local deployment but no active deployment exists."));
		return false;
	}

	bStoppingDeployment = true;

	FPlatformProcess::ClosePipe(0, ReadPipe);
	FPlatformProcess::ClosePipe(0, WritePipe);

	FPlatformProcess::TerminateProc(RuntimeProc, true);

	// bool bSuccess = SpatialCommandUtils::TryKillProcessWithPID(FString::Printf(TEXT("%u"), RuntimeProcID));

	if (!FPlatformProcess::IsProcRunning(RuntimeProc))
	{
		UE_LOG(LogSpatialDeploymentManager, Warning, TEXT("Stopped local runtime."));
	}
	else
	{
		UE_LOG(LogSpatialDeploymentManager, Error, TEXT("Failed to stop local runtime."));
	}

	FPlatformProcess::CloseProc(RuntimeProc);

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

// TODO: Change this to taking a snapshot without the services
void SPATIALGDKSERVICES_API FLocalDeploymentManager::TakeSnapshot(UWorld* World, bool bUseStandardRuntime,
																  FSpatialSnapshotTakenFunc OnSnapshotTaken)
{
	FHttpModule& HttpModule = FModuleManager::LoadModuleChecked<FHttpModule>("HTTP");
	TSharedRef<IHttpRequest> HttpRequest = HttpModule.Get().CreateRequest();

	HttpRequest->OnProcessRequestComplete().BindLambda(
		[World, bUseStandardRuntime, OnSnapshotTaken](FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded) {
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

			// Unfortunately by the time this callback happens, the files haven't been flushed, so if you copy you may get
			// the wrong info! So let's wait a bit..
			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(
				TimerHandle,
				[bUseStandardRuntime, OnSnapshotTaken]() {
					bool bSuccess = false;

					FString NewestSnapshotFilePath;
					FString SnapshotSearchDirectory;

					// Standard Runtime places the snapshots in SpatialOS logs, while Compatibility Mode Runtime
					// places them in AppData.
					if (bUseStandardRuntime)
					{
						FSpatialGDKServicesModule& GDKServices =
							FModuleManager::GetModuleChecked<FSpatialGDKServicesModule>("SpatialGDKServices");
						FLocalDeploymentManager* LocalDeploymentManager = GDKServices.GetLocalDeploymentManager();

						FString SpatialPath = SpatialGDKServicesConstants::SpatialOSDirectory;
						SnapshotSearchDirectory =
							SpatialPath + TEXT("logs/localdeployment/") + LocalDeploymentManager->GetLocalRunningDeploymentID();

						IFileManager& FileManager = FFileManagerGeneric::Get();

						// For Standard you need to check which is the latest .snapshot file that was added
						// inside that running deployment folder.
						TArray<FString> SnapshotFiles;

						FileManager.FindFiles(SnapshotFiles, *SnapshotSearchDirectory, TEXT("snapshot"));

						FDateTime NewestSnapshotTimestamp = FDateTime::MinValue();

						for (const FString& SnapshotFile : SnapshotFiles)
						{
							FString SnapshotFilePath = SnapshotSearchDirectory + TEXT("/") + SnapshotFile;
							FDateTime SnapshotFileTimestamp = FileManager.GetTimeStamp(*SnapshotFilePath);
							if (SnapshotFileTimestamp > NewestSnapshotTimestamp)
							{
								NewestSnapshotTimestamp = SnapshotFileTimestamp;
								NewestSnapshotFilePath = SnapshotFilePath;
							}
						}
					}
					else
					{
						// Compatibility Mode will have the latest created snapshot name in 'latest' file.
						FString AppDataLocalPath = FPlatformMisc::GetEnvironmentVariable(TEXT("LOCALAPPDATA"));
						SnapshotSearchDirectory = FString::Printf(TEXT("%s/.improbable/local_snapshots"), *AppDataLocalPath);
						FString LatestSnapshotInfoPath = FString::Printf(TEXT("%s/latest"), *SnapshotSearchDirectory);
						FString LatestSnapshot;
						if (FPaths::FileExists(LatestSnapshotInfoPath)
							&& FFileHelper::LoadFileToString(LatestSnapshot, *LatestSnapshotInfoPath))
						{
							NewestSnapshotFilePath =
								FString::Printf(TEXT("%s/.improbable/local_snapshots/%s"), *AppDataLocalPath, *LatestSnapshot);
						}
					}

					bSuccess = !NewestSnapshotFilePath.IsEmpty();

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

	if (bUseStandardRuntime)
	{
		HttpRequest->SetURL(TEXT("http://localhost:5006/snapshot"));
		HttpRequest->SetHeader("Content-Type", TEXT("application/json"));
		HttpRequest->SetVerb("GET");
	}
	else
	{
		HttpRequest->SetURL(TEXT("http://localhost:31000/improbable.platform.runtime.SnapshotService/TakeSnapshot"));
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/grpc-web+proto"));
		HttpRequest->SetVerb(TEXT("POST"));
		const TArray<uint8> Body = { 0, 0, 0, 0, 0 };
		HttpRequest->SetContent(Body);
	}
	HttpRequest->ProcessRequest();
}

#undef LOCTEXT_NAMESPACE
