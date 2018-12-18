// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Engine/EngineTypes.h"

#include "SpatialGDKEditorSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings, defaultconfig)
class SPATIALGDKEDITOR_API USpatialGDKEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer);

private:
	/** Path to the directory containing the SpatialOS-related files. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "SpatialOS directory"))
	FDirectoryPath SpatialOSDirectory;

private:
	/** Path to your SpatialOS snapshot. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot path"))
	FDirectoryPath SpatialOSSnapshotPath;

	/** Generated schema output path */
	UPROPERTY(EditAnywhere, config, Category = "Schema Generation", meta = (ConfigRestartRequired = false, DisplayName = "Output path for the generated schemas"))
	FDirectoryPath GeneratedSchemaOutputFolder;

public:
	FORCEINLINE FString GetSpatialOSDirectory() const
	{
		return SpatialOSDirectory.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::GetPath(FPaths::GetProjectFilePath()) + FString(TEXT("/../spatial/")))
			: SpatialOSDirectory.Path;
	}

	FORCEINLINE FString GetGeneratedSchemaOutputFolder() const
	{
		return GeneratedSchemaOutputFolder.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(GetSpatialOSDirectory(), FString(TEXT("schema/unreal/generated/"))))
			: GeneratedSchemaOutputFolder.Path;
	}

	FORCEINLINE FString GetSpatialOSSnapshotPath() const
	{
		return SpatialOSSnapshotPath.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(GetSpatialOSDirectory(), FString(TEXT("../spatial/snapshots/"))))
			: SpatialOSSnapshotPath.Path;
	}

	virtual FString ToString();
};
