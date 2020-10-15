// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageManager.h"
#include "SpatialGDKServicesConstants.h"

DEFINE_LOG_CATEGORY(LogSpatialPackageManager);

#define LOCTEXT_NAMESPACE "FSpatialPackageManager"

FSpatialPackageManager::FSpatialPackageManager() {}

void FSpatialPackageManager::Init() {}

bool FSpatialPackageManager::TryFetchRuntimeBinary()
{
	UE_LOG(LogSpatialPackageManager, Error, TEXT("UNR-4334 - Starting to fetch the Runtime binary"));

	FString RuntimePath = FString::Printf(TEXT("%s/runtime.exe"), *SpatialGDKServicesConstants::GDKProgramPath);

	// Check if the binary already exists for a given version

	bool bSuccess = FPaths::FileExists(RuntimePath);

	if (bSuccess) {
		UE_LOG(LogSpatialPackageManager, Error, TEXT("RUNTIME BINARIES ALREADY EXIST"));
	}

	// spatial package retrieve runtime x86_64-win32 0.5.1 runtime.zip
	else
	{
		FString RuntimeRetrieveArgs = FString::Printf(TEXT("package retrieve runtime x86_64-win32 0.5.1 runtime.zip"));
		FString RuntimeRetrieveResult;
		int32 ExitCode;
		FSpatialGDKServicesModule::ExecuteAndReadOutput(SpatialGDKServicesConstants::SpatialExe, RuntimeRetrieveArgs,
														SpatialGDKServicesConstants::GDKProgramPath, RuntimeRetrieveResult, ExitCode);
		const int32 exitCodeSuccess = 0;
		UE_LOG(LogSpatialPackageManager, Log, TEXT("OUTPUT %s"), *RuntimeRetrieveResult);
		return (ExitCode == exitCodeSuccess);
		
	}


	// If it does not exist then fetch the binary using `spatial worker package retrieve`
	// Download the zip to // UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Runtime\*version*

	// Unzip

	return true;
}

#undef LOCTEXT_NAMESPACE
