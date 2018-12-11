// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Engine/EngineTypes.h"

#include "SpatialGDKEditorToolbarSettings.generated.h"

USTRUCT()
struct FInteropTypebindingInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false))
	UClass* ReplicatedClass;

	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false))
	TArray<FString> IncludeList;
};

UCLASS(config = EditorPerProjectUserSettings, defaultconfig)
class USpatialGDKEditorToolbarSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKEditorToolbarSettings(const FObjectInitializer& ObjectInitializer);

private:
	/** Path to the directory containing the SpatialOS-related files. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "SpatialOS directory"))
	FDirectoryPath SpatialOSDirectory;
public:
	/** Launch configuration file used for `spatial local launch`. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Launch configuration"))
	FString SpatialOSLaunchConfig;

	/** Stop `spatial local launch` when shutting down editor. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Stop on exit"))
	bool bStopSpatialOnExit;

private:
	/** Path to your SpatialOS snapshot. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot path"))
	FDirectoryPath SpatialOSSnapshotPath;

	/** Name of your SpatialOS snapshot file. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot file name"))
	FString SpatialOSSnapshotFile;

	/** Generated schema output path */
	UPROPERTY(EditAnywhere, config, Category = "Schema Generation", meta = (ConfigRestartRequired = false, DisplayName = "Output path for the generated schemas"))
	FDirectoryPath GeneratedSchemaOutputFolder;

public:

	UFUNCTION()
	FORCEINLINE FString GetSpatialOSDirectory() const
	{
		return SpatialOSDirectory.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::GetPath(FPaths::GetProjectFilePath()) + FString(TEXT("/../spatial/")))
			: SpatialOSDirectory.Path;
	}

	UFUNCTION()
	FORCEINLINE FString GetGeneratedSchemaOutputFolder() const
	{
		return GeneratedSchemaOutputFolder.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(GetSpatialOSDirectory(), FString(TEXT("schema/unreal/generated/"))))
			: GeneratedSchemaOutputFolder.Path;
	}

	UFUNCTION()
	FORCEINLINE FString GetSpatialOSSnapshotPath() const
	{
		return SpatialOSSnapshotPath.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(GetSpatialOSDirectory(), FString(TEXT("../spatial/snapshots/"))))
			: SpatialOSSnapshotPath.Path;
	}

	UFUNCTION()
	FORCEINLINE FString GetSpatialOSSnapshotFile() const
	{
		return SpatialOSSnapshotFile.IsEmpty()
			? FString(TEXT("default.snapshot"))
			: SpatialOSSnapshotFile;
	}

	UFUNCTION()
	FString ToString();
};

