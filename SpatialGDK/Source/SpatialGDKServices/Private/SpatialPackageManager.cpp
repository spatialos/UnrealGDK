// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageManager.h"
#include "SpatialGDKServicesConstants.h"
#include "Async/Async.h"
#include "Misc\MonitoredProcess.h"

DEFINE_LOG_CATEGORY(LogSpatialPackageManager);

#define LOCTEXT_NAMESPACE "FSpatialPackageManager"

FSpatialPackageManager::FSpatialPackageManager() {}

void FSpatialPackageManager::Init() {}

bool FSpatialPackageManager::TryFetchRuntimeBinary(FString RuntimeVersion)
{

	UE_LOG(LogSpatialPackageManager, Log, TEXT("RUNTIME VERSION %s"), *RuntimeVersion);

	FString RuntimePath = FString::Printf(TEXT("%s/runtime/version-%s/%s"), *SpatialGDKServicesConstants::GDKProgramPath, *RuntimeVersion,
										  *SpatialGDKServicesConstants::RuntimeExe);

	// Check if the binary already exists for a given version
	if (FPaths::FileExists(RuntimePath))
	{
		UE_LOG(LogSpatialPackageManager, Log, TEXT("RUNTIME BINARIES ALREADY EXIST"));
	}

	// If it does not exist then fetch the binary using `spatial worker package retrieve`
	// Download the zip to // UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Runtime\*version* and unzip
	else
	{
		FString Params = FString::Printf(TEXT("package retrieve runtime x86_64-win32 %s runtime/version-%s --unzip"), *RuntimeVersion, *RuntimeVersion);
		FSpatialPackageManager FSpatialPackageManager = {};
		FSpatialPackageManager.StartProcess(Params, "Runtime Fetching");

	}
	
	return true;
}

bool FSpatialPackageManager::TryFetchInspectorBinary(FString InspectorVersion)
{

	FString InspectorPath = FString::Printf(TEXT("%s/inspector/version-%s/%s"), *SpatialGDKServicesConstants::GDKProgramPath, *InspectorVersion, *SpatialGDKServicesConstants::InspectorExe);

	// Check if the binary already exists
	if (FPaths::FileExists(InspectorPath))
	{
		UE_LOG(LogSpatialPackageManager, Log, TEXT("INSPECTOR BINARIES ALREADY EXIST"));
	}

	// If it does not exist then fetch the binary using `spatial worker package get`
	// Download the package to // UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs
	else
	{

		FString Params = FString::Printf(TEXT("package get inspector x86_64-win32 %s ./inspector/version-%s/%s"), *InspectorVersion,
										 *InspectorVersion, *SpatialGDKServicesConstants::InspectorExe);
		FSpatialPackageManager FSpatialPackageManager = {};
		FSpatialPackageManager.StartProcess(Params, "Inspector Fetching");

	}

	return true;
}

void FSpatialPackageManager::StartProcess(FString Params, FString ProcessName)
{
	FString WorkingDir = SpatialGDKServicesConstants::GDKProgramPath;
	auto ExePath = SpatialGDKServicesConstants::SpatialExe;
	FetchingProcess = {ExePath, Params, WorkingDir, true, true };
	bool IsStartingUp = true;
	FetchingProcess->OnOutput().BindLambda([&](const FString& Output) {
		UE_LOG(LogSpatialPackageManager, Display, TEXT("Runtime: %s"), *Output);
	});
	FetchingProcess->Launch();

	while (IsStartingUp && FetchingProcess->Update())
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
	auto Handle = FetchingProcess->GetProcessHandle();
	FPlatformProcess::TerminateProc(Handle);
}
#undef LOCTEXT_NAMESPACE
