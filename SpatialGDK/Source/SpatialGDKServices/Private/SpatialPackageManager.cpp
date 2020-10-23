// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Async/Async.h"
#include "SpatialPackageManager.h"
#include "SpatialGDKServicesConstants.h"


DEFINE_LOG_CATEGORY(LogSpatialPackageManager);


FSpatialPackageManager::FSpatialPackageManager() {}


void FSpatialPackageManager::TryFetchRuntimeBinary(FString RuntimeVersion)
{

	UE_LOG(LogSpatialPackageManager, Log, TEXT("Trying to fetch runtime version %s"), *RuntimeVersion);
	FString RuntimePath = FString::Printf(TEXT("%s/runtime/%s"), *SpatialGDKServicesConstants::GDKProgramPath, *RuntimeVersion);

	// Check if the binary already exists for a given version
	if (FPaths::FileExists(FString::Printf(TEXT("%s/%s"),*RuntimePath, *SpatialGDKServicesConstants::RuntimeExe)))
	{
		UE_LOG(LogSpatialPackageManager, Verbose, TEXT("Runtime binary already exist."));
		return;
	}

	// If it does not exist then fetch the binary using `spatial worker package retrieve`
	// Download the zip to // UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Runtime\*version* and unzip
	FString Params =
		FString::Printf(TEXT("package retrieve runtime %s %s %s --unzip"), *SpatialGDKServicesConstants::PlatformVersion, *RuntimeVersion, *RuntimePath);
	FSpatialPackageManager FSpatialPackageManager = {};
	FSpatialPackageManager.StartProcess(Params, "Runtime Fetching");
}

void FSpatialPackageManager::TryFetchInspectorBinary(FString InspectorVersion)
{

	FString InspectorPath = FString::Printf(TEXT("%s/inspector/%s"), *SpatialGDKServicesConstants::GDKProgramPath, *InspectorVersion);

	// Check if the binary already exists
	if (FPaths::FileExists(FString::Printf(TEXT("%s/%s"), *InspectorPath, *SpatialGDKServicesConstants::InspectorExe)))
	{
		UE_LOG(LogSpatialPackageManager, Log, TEXT("Inspector binaries already exist."));
		return;
	}

	// If it does not exist then fetch the binary using `spatial worker package get`
	// Download the package to // UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs
	FString Params = FString::Printf(TEXT("package get inspector %s %s %s/%s"), *SpatialGDKServicesConstants::PlatformVersion, *InspectorVersion,
									 *InspectorPath, *SpatialGDKServicesConstants::InspectorExe);
	FSpatialPackageManager FSpatialPackageManager = {};
	FSpatialPackageManager.StartProcess(Params, "Inspector Fetching");
}

void FSpatialPackageManager::StartProcess(FString Params, FString ProcessName)
{
	auto ExePath = SpatialGDKServicesConstants::SpatialExe;
	FetchingProcess = {ExePath, Params, true, true };
	FetchingProcess->OnOutput().BindLambda([&](const FString& Output) {
		UE_LOG(LogSpatialPackageManager, Display, TEXT("Runtime: %s"), *Output);
	});
	FetchingProcess->Launch();

	while (FetchingProcess->Update())
	{
		if (FetchingProcess->GetDuration().GetTotalSeconds() > 60)
		{
			UE_LOG(LogSpatialPackageManager, Error, TEXT("Timed out waiting for the %s fetching to start."));
			KillProcess(ProcessName);
			break;
		}
	}
}

void FSpatialPackageManager::KillProcess(FString ProcessName)
{
	UE_LOG(LogSpatialPackageManager, Warning, TEXT("Killing %s."), *ProcessName);
	if (FetchingProcess->Update())
	{
		auto Handle = FetchingProcess->GetProcessHandle();
		if (Handle.IsValid())
		{
			FPlatformProcess::TerminateProc(Handle);
		}
		else
		{
			UE_LOG(LogSpatialPackageManager, Error, TEXT("Killing %s was unsuccessful. Invalid Proc Handle."), *ProcessName);
		}
		
	}
}
