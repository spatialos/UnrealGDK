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

	FString RuntimePath = FString::Printf(TEXT("%s/runtime/runtime.exe"), *SpatialGDKServicesConstants::GDKProgramPath);

	// Check if the binary already exists for a given version
	bool bSuccess = FPaths::FileExists(RuntimePath);

	if (bSuccess)
	{
		UE_LOG(LogSpatialPackageManager, Log, TEXT("RUNTIME BINARIES ALREADY EXIST"));
	}

	// If it does not exist then fetch the binary using `spatial worker package retrieve`
	// Download the zip to // UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Runtime\*version* and unzip
	else
	{
		FString RuntimeRetrieveArgs = FString::Printf(TEXT("package retrieve runtime x86_64-win32 %s runtime --unzip"), *RuntimeVersion);
		FString RuntimeRetrieveResult;
		int32 ExitCode;
		FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialGDKServicesConstants::SpatialExe, RuntimeRetrieveArgs,
														SpatialGDKServicesConstants::GDKProgramPath, RuntimeRetrieveResult, ExitCode);
		const int32 exitCodeSuccess = 0;
		UE_LOG(LogSpatialPackageManager, Log, TEXT("OUTPUT %s"), *RuntimeRetrieveResult);
		return (ExitCode == exitCodeSuccess);
	}
	
	return true;
}

bool FSpatialPackageManager::TryFetchInspectorBinary()
{

	FString InspectorPath = FString::Printf(TEXT("%s/inspector.exe"), *SpatialGDKServicesConstants::GDKProgramPath);

	// Check if the binary already exists
	bool bSuccess = FPaths::FileExists(InspectorPath);

	if (bSuccess)
	{
		UE_LOG(LogSpatialPackageManager, Log, TEXT("INSPECTOR BINARIES ALREADY EXIST"));
	}

	// If it does not exist then fetch the binary using `spatial worker package get`
	// Download the exe to // UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs
	else
	{
		FString InspectorGetArgs = FString::Printf(TEXT("package get inspector x86_64-win32 1.1.2 ./inspector.exe"));
		FString InspectorGetResult;
		int32 ExitCode;
		FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialGDKServicesConstants::SpatialExe, InspectorGetArgs,
														SpatialGDKServicesConstants::GDKProgramPath, InspectorGetResult, ExitCode);
		const int32 exitCodeSuccess = 0;
		UE_LOG(LogSpatialPackageManager, Log, TEXT("OUTPUT %s"), *InspectorGetResult);
		return (ExitCode == exitCodeSuccess);
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
