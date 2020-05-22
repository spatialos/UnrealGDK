// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKServicesConstants.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialPackageService, Log, All);

// Represents a package that can be downloaded via the package service.
// This currently assumes that the package will extract into at least an executable with the same name as Type.
struct FSpatialPackageDescriptor
{
	const FString Type;
	const FString Name;
	const FString Version;

	FString ToString() const;

	// ExecutableDir + ExecutableName
	FString GetExecutablePath() const;
	// Directory that contains the package executable
	FString GetExecutableDir() const;
	// Name of the executable
	FString GetExecutableName() const;
};

class FSpatialPackageService
{
public:
	// Creates a package descriptor that can be used to download a runtime with the specified version
	SPATIALGDKSERVICES_API static FSpatialPackageDescriptor MakeRuntimeDescriptor(const FString& Version = SpatialGDKServicesConstants::SpatialOSSingleNodeRuntimePinnedVersion);

	// Ensures that the specified package is downloaded and extracted from the package service.
	// Does nothing if the package was already downloaded and extracted.
	// Callback will be executed with a true parameter if the package (now) exists at its extracted location, otherwise with false.
	SPATIALGDKSERVICES_API static void GetPackage(const FSpatialPackageDescriptor& Package, TFunction<void(bool)> Callback);
};
