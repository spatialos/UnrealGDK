// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorCloudLauncher.h"

#include "Interfaces/IPluginManager.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesModule.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorCloudLauncher);

namespace
{
	const FString LauncherExe = FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/DeploymentLauncher/DeploymentLauncher.exe"));
}

bool SpatialGDKCloudLaunch()
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();


	FString LauncherCreateArguments = FString::Printf(
		TEXT("create %s %s %s \"%s\" \"%s\" %s"),
		*FSpatialGDKServicesModule::GetProjectName(),
		*SpatialGDKSettings->GetAssemblyName(),
		*SpatialGDKSettings->GetPrimaryDeploymentName(),
		*SpatialGDKSettings->GetPrimaryLaunchConfigPath(),
		*SpatialGDKSettings->GetSnapshotPath(),
		*SpatialGDKSettings->GetPrimaryRegionCode().ToString()
	);

	if (SpatialGDKSettings->IsSimulatedPlayersEnabled())
	{
		LauncherCreateArguments = FString::Printf(
			TEXT("%s %s \"%s\" %s %s"),
			*LauncherCreateArguments,
			*SpatialGDKSettings->GetSimulatedPlayerDeploymentName(),
			*SpatialGDKSettings->GetSimulatedPlayerLaunchConfigPath(),
			*SpatialGDKSettings->GetSimulatedPlayerRegionCode().ToString(),
			*FString::FromInt(SpatialGDKSettings->GetNumberOfSimulatedPlayer())
		);
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

	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	// TODO: UNR-2435 - Add a Stop Deployment button and fix the code below:
	// get and provide deployment-id to stop the deployment as one of the LauncherStopArguments
	const FString LauncherStopArguments = FString::Printf(
		TEXT("stop %s"),
		*SpatialGDKSettings->GetPrimaryRegionCode().ToString()
		);

	int32 OutCode = 0;
	FString OutString;
	FString OutErr;

	bool bSuccess = FPlatformProcess::ExecProcess(*LauncherExe, *LauncherStopArguments, &OutCode, &OutString, &OutErr);
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
