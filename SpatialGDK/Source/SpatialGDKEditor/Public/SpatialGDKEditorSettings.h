// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/Paths.h"

#include "SpatialGDKEditorSettings.generated.h"

UCLASS(config = SpatialGDKEditorSettings, defaultconfig)
class SPATIALGDKEDITOR_API USpatialGDKEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer);

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostInitProperties() override;

private:
	/** Path to the directory containing the SpatialOS-related files. */
	UPROPERTY(EditAnywhere, config, Category = "General", meta = (ConfigRestartRequired = false, DisplayName = "SpatialOS directory"))
	FDirectoryPath SpatialOSDirectory;

public:
	/** If checked, all dynamically spawned entities will be deleted when server workers disconnect. */
	UPROPERTY(EditAnywhere, config, Category = "Play in editor settings", meta = (ConfigRestartRequired = false, DisplayName = "Delete Dynamically Spawned Entities"))
	bool bDeleteDynamicEntities;

	/** Generate default launch config. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (ConfigRestartRequired = false, DisplayName = "Generate default launch config"))
	bool bGenerateDefaultLaunchConfig;

private:
	/** Launch configuration file used for `spatial local launch`. */
	UPROPERTY(EditAnywhere, config, Category = "Launch", meta = (EditCondition = "!bGenerateDefaultLaunchConfig", ConfigRestartRequired = false, DisplayName = "Launch configuration"))
	FFilePath SpatialOSLaunchConfig;

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
	FFilePath SpatialOSSnapshotFile;

	/** Generated schema output path */
	UPROPERTY(EditAnywhere, config, Category = "Schema", meta = (ConfigRestartRequired = false, DisplayName = "Output path for the generated schemas"))
	FDirectoryPath GeneratedSchemaOutputFolder;

public:

	/** If checked, placeholder entities will be added to the snapshot on generation */
	UPROPERTY(EditAnywhere, config, Category = "Snapshots", meta = (ConfigRestartRequired = false, DisplayName = "Generate placeholder entities in snapshot"))
	bool bGeneratePlaceholderEntitiesInSnapshot;

	FORCEINLINE FString GetSpatialOSDirectory() const
	{
		return SpatialOSDirectory.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/")))
			: SpatialOSDirectory.Path;
	}

	FORCEINLINE FString GetSpatialOSLaunchConfig() const
	{
		return SpatialOSLaunchConfig.FilePath.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("/../spatial/default_launch.json")))
			: SpatialOSLaunchConfig.FilePath;
	}

	FORCEINLINE FString GetSpatialOSSnapshotFile() const
	{
		return SpatialOSSnapshotFile.FilePath.IsEmpty()
			? FPaths::ConvertRelativePathToFull(GetSpatialOSSnapshotFolderPath() + FString(TEXT("/default.snapshot")))
			: SpatialOSSnapshotFile.FilePath;
	}

	FORCEINLINE FString GetSpatialOSSnapshotFolderPath() const
	{
		return SpatialOSSnapshotPath.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(GetSpatialOSDirectory(), TEXT("../spatial/snapshots/")))
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
