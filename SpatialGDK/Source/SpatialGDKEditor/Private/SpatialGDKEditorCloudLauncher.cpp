// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorCloudLauncher.h"

#include "Interfaces/IPluginManager.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesModule.h"

bool SpatialGDKCloudLaunch()
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	const FString CmdExecutable = TEXT("cmd.exe");

	FString LauncherCmdArguments = FString::Printf(
		TEXT("/c cmd.exe /c DeploymentLauncher.exe create %s %s %s \"%s\" \"%s\" %s"),
		*FSpatialGDKServicesModule::GetProjectName(),
		*SpatialGDKSettings->GetAssemblyName(),
		*SpatialGDKSettings->GetPrimaryDeploymentName(),
		*SpatialGDKSettings->GetPrimaryLanchConfigPath(),
		*SpatialGDKSettings->GetSnapshotPath(),
		*SpatialGDKSettings->GetPrimaryRegionCode().ToString()
	);

	if (SpatialGDKSettings->IsSimulatedPlayersEnabled())
	{
		LauncherCmdArguments = FString::Printf(
			TEXT("%s %s \"%s\" %s %s"),
			*LauncherCmdArguments,
			*SpatialGDKSettings->GetSimulatedPlayerDeploymentName(),
			*SpatialGDKSettings->GetSimulatedPlayerLaunchConfigPath(),
			*SpatialGDKSettings->GetSimulatedPlayerRegionCode().ToString(),
			*FString::FromInt(SpatialGDKSettings->GetNumberOfSimulatedPlayer())
		);
	}

	LauncherCmdArguments = FString::Printf(
		TEXT("%s ^& pause"),
		*LauncherCmdArguments
	);

	FProcHandle DeploymentLauncherProcHandle = FPlatformProcess::CreateProc(
		*CmdExecutable, *LauncherCmdArguments, true, false, false, nullptr, 0,
		*SpatialGDKSettings->GetDeploymentLauncherPath(), nullptr, nullptr);

	if (DeploymentLauncherProcHandle.IsValid())
	{
		int32 ExitCode = 0;

		FPlatformProcess::WaitForProc(DeploymentLauncherProcHandle);
		FPlatformProcess::GetProcReturnCode(DeploymentLauncherProcHandle, &ExitCode);
		FPlatformProcess::CloseProc(DeploymentLauncherProcHandle);

		return (ExitCode == 0);
	}
	else
	{
		return false;
	}
}
 
bool SpatialGDKCloudStop()
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	const FString CmdExecutable = TEXT("cmd.exe");
	const FString LauncherCmdArguments = TEXT("/c DeploymentLauncher.exe stop");

	FProcHandle DeploymentLauncherProcHandle = FPlatformProcess::CreateProc(
		*CmdExecutable, *LauncherCmdArguments, true, false, false, nullptr, 0,
		*SpatialGDKSettings->GetDeploymentLauncherPath(), nullptr, nullptr);

	if (DeploymentLauncherProcHandle .IsValid())
	{
		int32 ExitCode = 0;

		FPlatformProcess::WaitForProc(DeploymentLauncherProcHandle);
		FPlatformProcess::GetProcReturnCode(DeploymentLauncherProcHandle, &ExitCode);
		FPlatformProcess::CloseProc(DeploymentLauncherProcHandle);

		return (ExitCode == 0);
	}
	else
	{
		return false;
	}
}
