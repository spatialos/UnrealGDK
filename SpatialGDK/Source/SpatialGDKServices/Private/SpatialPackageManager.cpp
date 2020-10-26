// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageManager.h"
#include "Async/Async.h"
#include "SpatialGDKServicesConstants.h"
#include "CoreMinimal.h"
#include "Misc/MonitoredProcess.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogSpatialPackageManager);

FSpatialPackageManager::FSpatialPackageManager() {}

void FSpatialPackageManager::FetchRuntimeBinary(const FString& RuntimeVersion)
{
	FString RuntimePath = FPaths::Combine(SpatialGDKServicesConstants::GDKProgramPath, TEXT("runtime"), RuntimeVersion);

	// Check if the binary already exists for a given version
	if (FPaths::FileExists(FPaths::Combine(RuntimePath, SpatialGDKServicesConstants::RuntimeExe)))
	{
		UE_LOG(LogSpatialPackageManager, Verbose, TEXT("Runtime binary already exists."));
		return;
	}

	// If it does not exist then fetch the binary using `spatial worker package retrieve`
	UE_LOG(LogSpatialPackageManager, Log, TEXT("Trying to fetch runtime version %s"), *RuntimeVersion);

	// Download and unzip the executable to "UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Runtime\*version*"
	FString Params =
		FString::Printf(TEXT("package retrieve runtime %s %s %s --unzip"), *SpatialGDKServicesConstants::PlatformVersion, *RuntimeVersion, *RuntimePath);
	FSpatialPackageManager FSpatialPackageManager;
	FSpatialPackageManager.StartProcess(Params, "Runtime Fetching");
}

void FSpatialPackageManager::FetchInspectorBinary(const FString& InspectorVersion)
{
	FString InspectorPath = FPaths::Combine(SpatialGDKServicesConstants::GDKProgramPath, TEXT("inspector"), InspectorVersion);

	// Check if the binary already exists
	if (FPaths::FileExists(FPaths::Combine (InspectorPath, SpatialGDKServicesConstants::InspectorExe)))
	{
		UE_LOG(LogSpatialPackageManager, Verbose, TEXT("Inspector binary already exists."));
		return;
	}

	// If it does not exist then fetch the binary using `spatial worker package get`
	// Download the package to UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs
	FString Params = FString::Printf(TEXT("package get inspector %s %s %s/%s"), *SpatialGDKServicesConstants::PlatformVersion, *InspectorVersion,
									 *InspectorPath, *SpatialGDKServicesConstants::InspectorExe);
	FSpatialPackageManager FSpatialPackageManager;
	FSpatialPackageManager.StartProcess(Params, "Inspector Fetching");
}

void FSpatialPackageManager::StartProcess(const FString& Params, const FString& ProcessName)
{
	const FString& ExePath = SpatialGDKServicesConstants::SpatialExe;
	FetchingProcess = {ExePath, Params, true, true };
	FetchingProcess->OnOutput().BindLambda([](const FString& Output) {
		UE_LOG(LogSpatialPackageManager, Display, TEXT("Runtime: %s"), *Output);
	});
	FetchingProcess->Launch();

	while (FetchingProcess->Update())
	{
		if (FetchingProcess->GetDuration().GetTotalSeconds() > SpatialGDKServicesConstants::ProcessTimeoutTime)
		{
			UE_LOG(LogSpatialPackageManager, Error, TEXT("Timed out waiting for the %s to start."), *ProcessName);
			KillProcess(ProcessName);
			break;
		}
	}
}

bool FSpatialPackageManager::KillProcess(const FString& ProcessName)
{
	UE_LOG(LogSpatialPackageManager, Warning, TEXT("Killing process %s."), *ProcessName);
	if (FetchingProcess->Update())
	{
		FProcHandle Handle = FetchingProcess->GetProcessHandle();
		if (Handle.IsValid())
		{
			FPlatformProcess::TerminateProc(Handle);
			return true;
		}
		UE_LOG(LogSpatialPackageManager, Error, TEXT("Killing process %s was unsuccessful. Invalid Proc Handle."), *ProcessName);
		return false;
	}
	UE_LOG(LogSpatialPackageManager, Error, TEXT("Killing process %s was unsuccessful. This process is not currently running."), *ProcessName);
	return false;
}
