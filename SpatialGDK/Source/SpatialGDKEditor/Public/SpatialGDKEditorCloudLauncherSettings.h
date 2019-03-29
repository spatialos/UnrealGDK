// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"
#include "SpatialGDKEditorSettings.h";

#include "SpatialGDKEditorCloudLauncherSettings.generated.h"

UCLASS(config = SpatialGDKEditorCloudLauncherSettings, defaultconfig)
class SPATIALGDKEDITOR_API USpatialGDKEditorCloudLauncherSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKEditorCloudLauncherSettings(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(EditAnywhere, config, Category = "General", meta = (ConfigRestartRequired = false, DisplayName = "SpatialOS project"))
	FString ProjectName;

	UPROPERTY(EditAnywhere, config, Category = "General", meta = (ConfigRestartRequired = false, DisplayName = "Assembly name"))
	FString AssemblyName;

	UPROPERTY(EditAnywhere, config, Category = "General", meta = (ConfigRestartRequired = false, DisplayName = "Deployment name"))
	FString PrimaryDeploymentName;

	UPROPERTY(EditAnywhere, config, Category = "General", meta = (ConfigRestartRequired = false, DisplayName = "Cloud launch configuration path"))
	FFilePath PrimaryLaunchConfigPath;

	UPROPERTY(EditAnywhere, config, Category = "General", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot path"))
	FFilePath SnapshotPath;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (ConfigRestartRequired = false, DisplayName = "Include simulated players"))
	bool bSimulatedPlayersIsEnabled;

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (EditCondition = "bSimulatedPlayersIsEnabled", ConfigRestartRequired = false, DisplayName = "Deployment mame"))
	FString SimulatedPlayerDeploymentName;

	const FString SimulatedPlayerLaunchConfigPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir() / TEXT("Plugins/UnrealGDK/SpatialGDK/Build/Programs/Improbable.Unreal.Scripts/WorkerCoordinator/SpatialConfig/cloud_launch_sim_player_deployment.json")));

	UPROPERTY(EditAnywhere, config, Category = "Simulated Players", meta = (EditCondition = "bSimulatedPlayersIsEnabled", ConfigRestartRequired = false, DisplayName = "Number of simulated players"))
	uint32 NumberOfSimulatedPlayers;

	static bool IsAssemblyNameValid(const FString& Name);
	static bool IsProjectNameValid(const FString& Name);
	static bool IsDeploymentNameValid(const FString& Name);

public:
	FString GetProjectNameFromSpatial() const;

	void SetPrimaryDeploymentName(const FString& Name);
	FORCEINLINE FString GetPrimaryDeploymentName() const
	{
		return PrimaryDeploymentName;
	}

	void SetAssemblyName(const FString& Name);
	FORCEINLINE FString GetAssemblyName() const
	{
		return AssemblyName;
	}

	void SetProjectName(const FString& Name);
	FORCEINLINE FString GetProjectName() const
	{
		return ProjectName;
	}

	void SetPrimaryLaunchConfigPath(const FString& Path);
	FORCEINLINE FString GetPrimaryLanchConfigPath() const
	{
		return PrimaryLaunchConfigPath.FilePath;
	}

	void SetSnapshotPath(const FString& Path);
	FORCEINLINE FString GetSnapshotPath() const
	{
		const USpatialGDKEditorSettings* SpatialEditorSettings = GetDefault<USpatialGDKEditorSettings>();
		return SnapshotPath.FilePath.IsEmpty()
			? FPaths::Combine(GetDefault<USpatialGDKEditorSettings>()->GetSpatialOSSnapshotFolderPath(), GetDefault<USpatialGDKEditorSettings>()->GetSpatialOSSnapshotFile())
			: SnapshotPath.FilePath;
	}

	void SetSimulatedPlayersEnabledState(bool IsEnabled);
	FORCEINLINE bool IsSimulatedPlayersEnabled() const
	{
		return bSimulatedPlayersIsEnabled;
	}

	void SetSimulatedPlayerDeploymentName(const FString& Name);
	FORCEINLINE FString GetSimulatedPlayerDeploymentName() const
	{
		return SimulatedPlayerDeploymentName;
	}

	FORCEINLINE FString GetSimulatedPlayerLaunchConfigPath() const
	{
		return SimulatedPlayerLaunchConfigPath;
	}

	void SetNumberOfSimulatedPlayers(uint32 Number);
	FORCEINLINE uint32 GetNumberOfSimulatedPlayer() const
	{
		return NumberOfSimulatedPlayers;
	}

	bool IsDeploymentConfigurationValid() const;
};
