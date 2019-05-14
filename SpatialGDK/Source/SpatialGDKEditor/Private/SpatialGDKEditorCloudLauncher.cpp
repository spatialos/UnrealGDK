// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorCloudLauncher.h"

#include "Interfaces/IPluginManager.h"
#include "SpatialGDKEditorCloudLauncherSettings.h"
#include "SpatialGDKEditorSettings.h"

bool SpatialGDKCloudLaunch()
{
	const USpatialGDKEditorCloudLauncherSettings* SpatialGDKCloudLauncherSettings = GetDefault<USpatialGDKEditorCloudLauncherSettings>();
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	const FString CmdExecutable = TEXT("cmd.exe");

	FString LauncherCmdArguments = FString::Printf(
		TEXT("/c DeploymentLauncher.exe create %s %s %s %s %s %s"),
		*SpatialGDKCloudLauncherSettings->GetProjectName(),
		*SpatialGDKCloudLauncherSettings->GetAssemblyName(),
		*SpatialGDKCloudLauncherSettings->GetPrimaryDeploymentName(),
		*SpatialGDKCloudLauncherSettings->GetPrimaryLanchConfigPath(),
		*SpatialGDKCloudLauncherSettings->GetSnapshotPath(),
		*SpatialGDKCloudLauncherSettings->GetPrimaryRegionCode().ToString()
	);

	if (SpatialGDKCloudLauncherSettings->IsSimulatedPlayersEnabled())
	{
		LauncherCmdArguments = FString::Printf(
			TEXT("%s %s %s %s %s"),
			*LauncherCmdArguments,
			*SpatialGDKCloudLauncherSettings->GetSimulatedPlayerDeploymentName(),
			*SpatialGDKCloudLauncherSettings->GetSimulatedPlayerLaunchConfigPath(),
			*SpatialGDKCloudLauncherSettings->GetSimulatedPlayerRegionCode().ToString(),
			*FString::FromInt(SpatialGDKCloudLauncherSettings->GetNumberOfSimulatedPlayer())
		);
	}

	FProcHandle DeploymentLauncherProcHandle = FPlatformProcess::CreateProc(
		*CmdExecutable, *LauncherCmdArguments, true, false, false, nullptr, 0,
		*DeploymentLauncherAbsolutePath, nullptr, nullptr);

	return DeploymentLauncherProcHandle.IsValid();
}
 
bool SpatialGDKCloudStop()
{
	const USpatialGDKEditorCloudLauncherSettings* SpatialGDKCloudLauncherSettings = GetDefault<USpatialGDKEditorCloudLauncherSettings>();
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	const FString CmdExecutable = TEXT("cmd.exe");
	const FString LauncherCmdArguments = TEXT("/c DeploymentLauncher.exe stop");

	FProcHandle DeploymentLauncherProcHandle = FPlatformProcess::CreateProc(
		*CmdExecutable, *LauncherCmdArguments, true, false, false, nullptr, 0,
		*DeploymentLauncherAbsolutePath, nullptr, nullptr);

	return DeploymentLauncherProcHandle.IsValid();
}
