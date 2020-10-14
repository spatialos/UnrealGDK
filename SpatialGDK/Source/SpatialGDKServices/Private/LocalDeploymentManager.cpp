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

	if (bCommandletRunning)
	{
		bLocalDeploymentManagerEnabled = false;
		return;
	}

	// Ensure the worker.jsons are up to date.
	// TOOD: Probably remove this
	WorkerBuildConfigAsync();

	// Watch the worker config directory for changes.
	StartUpWorkerConfigDirectoryWatcher();
}

void FLocalDeploymentManager::Init(FString RuntimeIPToExpose) {}

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

	return bSuccess;
}

// TODO: Change this to starting the runtime binary
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

// TODO: Might not need all these additional checks now
void FLocalDeploymentManager::TryStartLocalDeployment(FString LaunchConfig, FString RuntimeVersion, FString LaunchArgs,
													  FString SnapshotName, FString RuntimeIPToExpose,
													  const LocalDeploymentCallback& CallBack)
{
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

	// TODO: Pretty sure we don't need this runtime exposing code anymore
	//// Stop the currently running service if the runtime IP is to be exposed, but is different from the one specified
	// if (ExposedRuntimeIP != RuntimeIPToExpose)
	//{
	//	UE_LOG(LogSpatialDeploymentManager, Verbose,
	//		   TEXT("Settings for exposing runtime IP have changed since service startup. Restarting service to reflect changes."));
	//}

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

// TODO: Change this to killing the runtime binary
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
