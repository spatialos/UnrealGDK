// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageManager.h"
#include "SpatialGDKServicesConstants.h"


DEFINE_LOG_CATEGORY(LogSpatialPackageManager);

#define LOCTEXT_NAMESPACE "FSpatialPackageManager"

FSpatialPackageManager::FSpatialPackageManager() {}

void FSpatialPackageManager::Init() {}

bool FSpatialPackageManager::TryFetchRuntimeBinary(FString RuntimeVersion)
{

	UE_LOG(LogSpatialPackageManager, Log, TEXT("RUNTIME VERSION %s"), *RuntimeVersion);

	FString RuntimePath = FString::Printf(TEXT("%s/runtime/version-%s/runtime.exe"), *SpatialGDKServicesConstants::GDKProgramPath, *RuntimeVersion);

	// Check if the binary already exists for a given version
	if (FPaths::FileExists(RuntimePath))
	{
		UE_LOG(LogSpatialPackageManager, Log, TEXT("RUNTIME BINARIES ALREADY EXIST"));
	}

	// If it does not exist then fetch the binary using `spatial worker package retrieve`
	// Download the zip to // UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Runtime\*version* and unzip
	else
	{
		FString RuntimeRetrieveArgs =
			FString::Printf(TEXT("package retrieve runtime x86_64-win32 %s runtime/version-%s --unzip"), *RuntimeVersion, *RuntimeVersion);
		FString RuntimeRetrieveResult;
		int32 ExitCode;
		FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialGDKServicesConstants::SpatialExe, RuntimeRetrieveArgs,
														SpatialGDKServicesConstants::GDKProgramPath, RuntimeRetrieveResult, ExitCode);
		UE_LOG(LogSpatialPackageManager, Log, TEXT("OUTPUT %s"), *RuntimeRetrieveResult);
		return (ExitCode == ExitCodeSuccess);
	}
	
	return true;
}

bool FSpatialPackageManager::TryFetchInspectorBinary(FString InspectorVersion)
{

	FString InspectorPath = FString::Printf(TEXT("%s/inspector/version-%s/inspector.exe"), *SpatialGDKServicesConstants::GDKProgramPath, *InspectorVersion);

	// Check if the binary already exists
	if (FPaths::FileExists(InspectorPath))
	{
		UE_LOG(LogSpatialPackageManager, Log, TEXT("INSPECTOR BINARIES ALREADY EXIST"));
	}

	// If it does not exist then fetch the binary using `spatial worker package get`
	// Download the package to // UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs
	else
	{
		FString InspectorGetArgs = FString::Printf(TEXT("package get inspector x86_64-win32 %s ./inspector/version-%s/inspector.exe"),
												   *InspectorVersion, *InspectorVersion);
		FString InspectorGetResult;
		int32 ExitCode;
		FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialGDKServicesConstants::SpatialExe, InspectorGetArgs,
														SpatialGDKServicesConstants::GDKProgramPath, InspectorGetResult, ExitCode);
		UE_LOG(LogSpatialPackageManager, Log, TEXT("OUTPUT %s"), *InspectorGetResult);
		return (ExitCode == ExitCodeSuccess);
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
