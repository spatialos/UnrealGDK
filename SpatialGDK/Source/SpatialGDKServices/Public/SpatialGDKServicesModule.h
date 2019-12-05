// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LocalDeploymentManager.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class SPATIALGDKSERVICES_API FSpatialGDKServicesModule : public IModuleInterface
{
public:
	static FString ProjectName;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

	FLocalDeploymentManager* GetLocalDeploymentManager();

	static FString GetSpatialOSDirectory(const FString& AppendPath = TEXT(""));
	static FString GetSpatialGDKPluginDirectory(const FString& AppendPath = TEXT(""));
	static const FString& GetSpotExe();
	static const FString& GetSpatialExe();
	static bool SpatialPreRunChecks();

	FORCEINLINE static FString GetProjectName()
	{
		return ProjectName;
	}

	static bool ParseJson(const FString& RawJsonString, TSharedPtr<FJsonObject>& JsonParsed);
	static void ExecuteAndReadOutput(const FString& Executable, const FString& Arguments, const FString& DirectoryToRun, FString& OutResult, int32& ExitCode);

private:
	FLocalDeploymentManager LocalDeploymentManager;
	static FString ParseProjectName();
};
