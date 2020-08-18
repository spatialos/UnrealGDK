// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LocalDeploymentManager.h"
#include "LocalReceptionistProxyServerManager.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class SPATIALGDKSERVICES_API FSpatialGDKServicesModule : public IModuleInterface
{
public:
	static FString ProjectName;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override { return true; }

	FLocalDeploymentManager* GetLocalDeploymentManager();
	FLocalReceptionistProxyServerManager* GetLocalReceptionistProxyServerManager();

	static FString GetSpatialGDKPluginDirectory(const FString& AppendPath = TEXT(""));

	static bool SpatialPreRunChecks(bool bIsInChina);

	FORCEINLINE static FString GetProjectName() { return ProjectName; }

	static void SetProjectName(const FString& InProjectName);

	static bool ParseJson(const FString& RawJsonString, TSharedPtr<FJsonObject>& JsonParsed);
	static void ExecuteAndReadOutput(const FString& Executable, const FString& Arguments, const FString& DirectoryToRun, FString& OutResult,
									 int32& ExitCode);

private:
	FLocalDeploymentManager LocalDeploymentManager;
	FLocalReceptionistProxyServerManager LocalReceptionistProxyServerManager;

	static FString ParseProjectName();
	static TSharedPtr<FJsonObject> ParseProjectFile();
};
