// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"

#include "SpatialGDKEditorCloudLauncherSettings.generated.h"

UCLASS(config = SpatialGDKEditorCloudLauncherSettings, defaultconfig)
class SPATIALGDKEDITOR_API USpatialGDKEditorCloudLauncherSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKEditorCloudLauncherSettings(const FObjectInitializer& ObjectInitializer);

private:
	/** Path to the directory containing the SpatialOS-related files. */
	UPROPERTY(EditAnywhere, config, Category = "General", meta = (ConfigRestartRequired = false, DisplayName = "SpatialOS directory"))
		FDirectoryPath SpatialOSDirectory;

private:
	UPROPERTY(config)
	FString ProjectName;

	UPROPERTY(config)
	FString PrimaryDeploymentName;

	UPROPERTY(config)
	FString AssemblyName;

	UPROPERTY(config)
	FFilePath PrimaryLaunchConfigPath;

	UPROPERTY(config)
	FFilePath SnapshotPath;

	UPROPERTY(config)
	FString SimulatedPlayerDeploymentName;

	UPROPERTY(config)
	FFilePath SimulatedPlayerLaunchConfigPath;

	UPROPERTY(config)
	uint32 NumberOfSimulatedPlayers;

	bool PrimaryDeploymentNameIsValid;
	bool AssemblyNameIsValid;
	bool ProjectNameIsValid;

	UPROPERTY(config)
	bool SimulatedPlayersIsEnabled;

	void ValidateAssemblyName();
	void ValidateProjectName();
	void ValidateDeploymentName();

public:
	FString GetProjectNameFromSpatial();

	void SetPrimaryDeploymentName(const FString & Name);
	FORCEINLINE FString GetPrimaryDeploymentName() const
	{
		return PrimaryDeploymentName;
	}

	void SetAssemblyName(const FString & Name);
	FORCEINLINE FString GetAssemblyName() const
	{
		return AssemblyName;
	}

	void SetProjectName(const FString & Name);
	FORCEINLINE FString GetProjectName() const
	{
		return ProjectName;
	}

	void SetPrimaryLaunchConfigPath(const FString & Path);
	FORCEINLINE FString GetPrimaryLanchConfigPath() const
	{
		return PrimaryLaunchConfigPath.FilePath;
	}

	void SetSnapshotPath(const FString & Path);
	FORCEINLINE FString GetSnapshotPath() const
	{
		return SnapshotPath.FilePath;
	}

	void SetSimulatedPlayersEnabledState(bool IsEnabled);
	FORCEINLINE bool IsSimulatedPlayersEnabled() const
	{
		return SimulatedPlayersIsEnabled;
	}

	void SetSimulatedPlayerDeploymentName(const FString & Name);
	FORCEINLINE FString GetSimulatedPlayerDeploymentName() const
	{
		return SimulatedPlayerDeploymentName;
	}

	void SetSimulatedPlayerLaunchConfigPath(const FString & Path);
	FORCEINLINE FString GetSimulatedPlayerLaunchConfigPath() const
	{
		return SimulatedPlayerLaunchConfigPath.FilePath;
	}

	void SetNumberOfSimulatedPlayers(uint32 Number);
	FORCEINLINE uint32 GetNumberOfSimulatedPlayer() const
	{
		return NumberOfSimulatedPlayers;
	}

	FORCEINLINE bool IsProjectNameValid() const
	{
		return ProjectNameIsValid;
	}

	FORCEINLINE bool IsPrimaryDeploymentNameValid() const
	{
		return PrimaryDeploymentNameIsValid;
	}

	FORCEINLINE bool IsAssemblyNameValid() const
	{
		return AssemblyNameIsValid;
	}

	bool IsDeploymentConfigurationValidSinceLastCheck() const;

	bool IsDeploymentConfigurationValidWithCheck() const;

	virtual FString ToString();
};
