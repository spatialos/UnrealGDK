// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorCloudLauncher.h"

#include "GenericPlatform/GenericPlatformProcess.h"

#include "CloudDeploymentConfiguration.h"
#include "SpatialGDKServicesModule.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorCloudLauncher);

namespace
{
const FString LauncherExe = FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(
	TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/DeploymentLauncher/DeploymentLauncher.exe"));
}

bool SpatialGDKCloudLaunch(const FCloudDeploymentConfiguration& Configuration)
{
	FString LauncherCreateArguments =
		FString::Printf(TEXT("create %s %s %s %s \"%s\" \"%s\" %s \"%s\" \"%s\""), *FSpatialGDKServicesModule::GetProjectName(),
						*Configuration.AssemblyName, *Configuration.RuntimeVersion, *Configuration.PrimaryDeploymentName,
						*Configuration.PrimaryLaunchConfigPath, *Configuration.SnapshotPath, *Configuration.PrimaryRegionCode,
						*Configuration.MainDeploymentCluster, *Configuration.DeploymentTags);

	if (Configuration.bSimulatedPlayersEnabled)
	{
		LauncherCreateArguments =
			FString::Printf(TEXT("%s %s \"%s\" %s \"%s\" %u"), *LauncherCreateArguments, *Configuration.SimulatedPlayerDeploymentName,
							*Configuration.SimulatedPlayerLaunchConfigPath, *Configuration.SimulatedPlayerRegionCode,
							*Configuration.SimulatedPlayerCluster, Configuration.NumberOfSimulatedPlayers);
	}

	if (Configuration.bUseChinaPlatform)
	{
		LauncherCreateArguments += TEXT(" --china");
	}

	int32 OutCode = 0;
	FString OutString;
	FString OutErr;

	bool bSuccess = FPlatformProcess::ExecProcess(*LauncherExe, *LauncherCreateArguments, &OutCode, &OutString, &OutErr);
	if (OutCode != 0)
	{
		UE_LOG(LogSpatialGDKEditorCloudLauncher, Error, TEXT("Cloud Launch failed with code %d: %s"), OutCode, *OutString);
		if (!OutErr.IsEmpty())
		{
			UE_LOG(LogSpatialGDKEditorCloudLauncher, Error, TEXT("%s"), *OutErr);
		}
		bSuccess = false;
	}

	return bSuccess;
}

bool SpatialGDKCloudStop()
{
	UE_LOG(LogSpatialGDKEditorCloudLauncher, Error, TEXT("Function not available"));
	return false;

	// TODO: UNR-2435 - Add a Stop Deployment button and fix the code below:
	// get and provide deployment-id to stop the deployment as one of the LauncherStopArguments
	int32 OutCode = 0;
	FString OutString;
	FString OutErr;

	bool bSuccess = FPlatformProcess::ExecProcess(*LauncherExe, TEXT("stop"), &OutCode, &OutString, &OutErr);
	if (OutCode != 0)
	{
		UE_LOG(LogSpatialGDKEditorCloudLauncher, Error, TEXT("Cloud Launch failed with code %d: %s"), OutCode, *OutString);
		if (!OutErr.IsEmpty())
		{
			UE_LOG(LogSpatialGDKEditorCloudLauncher, Error, TEXT("%s"), *OutErr);
		}
		bSuccess = false;
	}

	return bSuccess;
}
