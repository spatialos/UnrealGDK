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

	// Check if the binary already exists for a given version

	// UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs
	FString RuntimePath = SpatialGDKServicesConstants::GDKProgramPath;

	// If it does not exist then fetch the binary using `spatial worker package retrieve`
	// Download the zip to // UnrealGDK\SpatialGDK\Binaries\ThirdParty\Improbable\Programs\Runtime\*version*

	// Unzip

	return true;
}

#undef LOCTEXT_NAMESPACE
