// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorCloudLauncher.h"

#include "Interfaces/IPluginManager.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesModule.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKEditorCloudLauncher);

bool SpatialGDKCloudLaunch()
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	const FString LauncherExe = FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/DeploymentLauncher/DeploymentLauncher.exe"));

	FString CmdArguments = FString::Printf(
		TEXT("create %s %s %s \"%s\" \"%s\" %s"),
		*FSpatialGDKServicesModule::GetProjectName(),
		*SpatialGDKSettings->GetAssemblyName(),
		*SpatialGDKSettings->GetPrimaryDeploymentName(),
		*SpatialGDKSettings->GetPrimaryLanchConfigPath(),
		*SpatialGDKSettings->GetSnapshotPath(),
		*SpatialGDKSettings->GetPrimaryRegionCode().ToString()
	);

	if (SpatialGDKSettings->IsSimulatedPlayersEnabled())
	{
		CmdArguments = FString::Printf(
			TEXT("%s %s \"%s\" %s %s"),
			*CmdArguments,
			*SpatialGDKSettings->GetSimulatedPlayerDeploymentName(),
			*SpatialGDKSettings->GetSimulatedPlayerLaunchConfigPath(),
			*SpatialGDKSettings->GetSimulatedPlayerRegionCode().ToString(),
			*FString::FromInt(SpatialGDKSettings->GetNumberOfSimulatedPlayer())
		);
	}

	int32 OutCode = 0;
	FString OutString;
	FString OutErr;

	bool bSuccess = FPlatformProcess::ExecProcess(*LauncherExe, *CmdArguments, &OutCode, &OutString, &OutErr);
	if (OutCode != 0)
	{
		UE_LOG(LogSpatialGDKEditorCloudLauncher, Warning, TEXT("Cloud Launch failed: %s : %s"), *OutString, *OutErr);
		bSuccess = false;
	}

	return bSuccess;
}
 
bool SpatialGDKCloudStop()
{
	UE_LOG(LogSpatialGDKEditorCloudLauncher, Warning, TEXT("Function not available"));
	return false;

	// TODO: UNR-2435 - Add a Stop Deployment button and fix the code below:
	// get and provide deployment-id to stop the deployment as one of the CmdArguments
	const FString CmdArguments = TEXT("stop");
	const FString LauncherExe = FSpatialGDKServicesModule::GetSpatialGDKPluginDirectory(TEXT("SpatialGDK/Binaries/ThirdParty/Improbable/Programs/DeploymentLauncher/DeploymentLauncher.exe"));

	int32 OutCode = 0;
	FString OutString;
	FString OutErr;

	bool bSuccess = FPlatformProcess::ExecProcess(*LauncherExe, *CmdArguments, &OutCode, &OutString, &OutErr);
	if (OutCode != 0)
	{
		UE_LOG(LogSpatialGDKEditorCloudLauncher, Warning, TEXT("Cloud Launch failed: %s : %s"), *OutString, *OutErr);
		bSuccess = false;
	}

	return bSuccess;
}
