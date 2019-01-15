// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"

#include "SpatialGDKEditorSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings, defaultconfig)
class SPATIALGDKEDITOR_API USpatialGDKEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer);

private:
	/** Path to the directory containing the SpatialOS-related files. */
	UPROPERTY(EditAnywhere, config, Category = "General", meta = (ConfigRestartRequired = false, DisplayName = "SpatialOS directory"))
	FDirectoryPath SpatialOSDirectory;

	/** Launch configuration file used for `spatial local launch`. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (ConfigRestartRequired = false, DisplayName = "Launch configuration"))
	FString SpatialOSLaunchConfig;

public:
	/** Stop `spatial local launch` when shutting down editor. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (ConfigRestartRequired = false, DisplayName = "Stop on exit"))
	bool bStopSpatialOnExit;

private:
	/** Path to your SpatialOS snapshot. */
	UPROPERTY(EditAnywhere, config, Category = "Snapshots", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot path"))
	FDirectoryPath SpatialOSSnapshotPath;

	/** Name of your SpatialOS snapshot file. */
	UPROPERTY(EditAnywhere, config, Category = "Snapshots", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot file name"))
	FString SpatialOSSnapshotFile;

	/** Generated schema output path */
	UPROPERTY(EditAnywhere, config, Category = "Schema", meta = (ConfigRestartRequired = false, DisplayName = "Output path for the generated schemas"))
	FDirectoryPath GeneratedSchemaOutputFolder;

public:
	FORCEINLINE FString GetSpatialOSDirectory() const
	{
		return SpatialOSDirectory.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::GetPath(FPaths::GetProjectFilePath()) + FString(TEXT("/../spatial/")))
			: SpatialOSDirectory.Path;
	}

	FORCEINLINE FString GetSpatialOSLaunchConfig() const
	{
		return SpatialOSLaunchConfig.IsEmpty() ? FString(TEXT("default_launch.json")) : SpatialOSLaunchConfig;
	}

	UFUNCTION()
	FORCEINLINE FString GetSpatialOSSnapshotFile() const
	{
		return SpatialOSSnapshotFile.IsEmpty() ? FString(TEXT("default.snapshot")) : SpatialOSSnapshotFile;
	}

	FORCEINLINE FString GetSpatialOSSnapshotPath() const
	{
		return SpatialOSSnapshotPath.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(GetSpatialOSDirectory(), FString(TEXT("../spatial/snapshots/"))))
			: SpatialOSSnapshotPath.Path;
	}

	FORCEINLINE FString GetGeneratedSchemaOutputFolder() const
	{
		return GeneratedSchemaOutputFolder.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(GetSpatialOSDirectory(), FString(TEXT("schema/unreal/generated/"))))
			: GeneratedSchemaOutputFolder.Path;
	}
	
	virtual FString ToString();
};
