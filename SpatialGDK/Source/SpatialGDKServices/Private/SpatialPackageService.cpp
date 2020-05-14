// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageService.h"

#include "Async/Async.h"
#include "HAL/FileManager.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"

DEFINE_LOG_CATEGORY(LogSpatialPackageService);

namespace
{
	bool DownloadPackage(const FSpatialPackageDescriptor& Package)
	{
		UE_LOG(LogSpatialPackageService, Log, TEXT("Downloading package: '%s'"), *Package.ToString());

		const FString Args = FString::Printf(TEXT("package retrieve %s %s %s %s --unzip"),
			*Package.Type, *Package.Name, *Package.Version, *Package.GetExecutableDir());

		FString Result;
		int32 ExitCode;
		FSpatialGDKServicesModule::ExecuteAndReadOutput(*SpatialGDKServicesConstants::SpatialExe, Args,
            *SpatialGDKServicesConstants::GDKProgramPath, Result, ExitCode);

		if (ExitCode != 0)
		{
			UE_LOG(LogSpatialPackageService, Error, TEXT("Could not download package '%s' from package service"), *Package.ToString());
			return false;
		}

		UE_LOG(LogSpatialPackageService, Log, TEXT("Downloaded package: %s"), *Result);
		return true;
	}
}

FString FSpatialPackageDescriptor::ToString() const
{
	return FString::Printf(TEXT("%s %s %s"), *Type, *Name, *Version);
}

FString FSpatialPackageDescriptor::GetExecutable() const
{
	return FString::Printf(TEXT("%s/%s"), *GetExecutableDir(), *GetExecutableName());
}

FString FSpatialPackageDescriptor::GetExecutableDir() const
{
	return FString::Printf(TEXT("%s/%s/%s"), *SpatialGDKServicesConstants::GDKProgramPath, *Type, *Version);
}

FString FSpatialPackageDescriptor::GetExecutableName() const
{
#ifdef PLATFORM_WINDOWS
	return FString::Printf(TEXT("%s.exe"), *Type);
#endif
}

FSpatialPackageDescriptor FSpatialPackageService::MakeRuntimeDescriptor(const FString& Version)
{
#ifdef PLATFORM_WINDOWS
	return FSpatialPackageDescriptor{"runtime", "x86_64-win32", Version};
#endif
}

void FSpatialPackageService::GetPackage(const FSpatialPackageDescriptor& Package, TFunction<void(bool)> Callback)
{
	if (!IFileManager::Get().FileExists(*Package.GetExecutable()))
	{
		AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [Package, Callback]
		{
			const bool Result = DownloadPackage(Package);
			Callback(Result);
		});
	}
	else
	{
		Callback(true);
	}
}
