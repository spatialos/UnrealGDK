#include "SpatialGDKEditorCloudLauncher.h"

#include "Interfaces/IPluginManager.h"
#include "SpatialGDKEditorCloudLauncherSettings.h"
#include "SpatialGDKEditorSettings.h"

bool SpatialGDKCloudLaunch()
{
	const USpatialGDKEditorCloudLauncherSettings* SpatialGDKCloudLauncherSettings = GetDefault<USpatialGDKEditorCloudLauncherSettings>();
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	const FString ExecuteAbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir() / TEXT("Plugins/UnrealGDK/SpatialGDK/Binaries/ThirdParty/Improbable/Programs/DeploymentLauncher")));
	const FString CmdExecutable = TEXT("cmd.exe");

	uint32 DeploymentLauncherProcessID;

	FString LauncherCmdArguments = FString::Printf(
		TEXT("/c cmd.exe /c DeploymentLauncher.exe create %s %s %s %s %s "),
		*SpatialGDKCloudLauncherSettings->GetProjectName(),
		*SpatialGDKCloudLauncherSettings->GetAssemblyName(),
		*SpatialGDKCloudLauncherSettings->GetPrimaryDeploymentName(),
		*SpatialGDKCloudLauncherSettings->GetPrimaryLanchConfigPath(),
		*SpatialGDKCloudLauncherSettings->GetSnapshotPath()
	);

	if (SpatialGDKCloudLauncherSettings->IsSimulatedPlayersEnabled())
	{
		LauncherCmdArguments = FString::Printf(
			TEXT("%s %s %s %s"),
			*LauncherCmdArguments,
			*SpatialGDKCloudLauncherSettings->GetSimulatedPlayerDeploymentName(),
			*SpatialGDKCloudLauncherSettings->GetSimulatedPlayerLaunchConfigPath(),
			*FString::FromInt(SpatialGDKCloudLauncherSettings->GetNumberOfSimulatedPlayer())
		);
	}

	FProcHandle DeploymentLauncherProcHandle = FPlatformProcess::CreateProc(
		*(CmdExecutable), *LauncherCmdArguments, true, false, false, &DeploymentLauncherProcessID, 0,
		*ExecuteAbsolutePath, nullptr, nullptr);

	return DeploymentLauncherProcHandle.IsValid();
}
 
bool SpatialGDKCloudStop()
{
	const USpatialGDKEditorCloudLauncherSettings* SpatialGDKCloudLauncherSettings = GetDefault<USpatialGDKEditorCloudLauncherSettings>();
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	const FString ExecuteAbsolutePath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir() / TEXT("Plugins/UnrealGDK/SpatialGDK/Binaries/ThirdParty/Improbable/Programs/DeploymentLauncher")));
	const FString CmdExecutable = TEXT("cmd.exe");

	uint32 DeploymentLauncherProcessID;

	const FString LauncherCmdArguments = TEXT("/c cmd.exe /c DeploymentLauncher.exe stop");

	FProcHandle DeploymentLauncherProcHandle = FPlatformProcess::CreateProc(
		*(CmdExecutable), *LauncherCmdArguments, true, false, false, &DeploymentLauncherProcessID, 0,
		*ExecuteAbsolutePath, nullptr, nullptr);

	return DeploymentLauncherProcHandle.IsValid();
}
