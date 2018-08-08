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

	/** Root folder of your SpatialOS Unreal GDK project. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false))
	FDirectoryPath ProjectRootFolder;

	/** Launch configuration file used for `spatial local launch`. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Launch Configuration"))
	FString SpatialOSLaunchConfig;

	/** Stop `spatial local launch` when shutting down editor. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Stop on Exit"))
	bool bStopSpatialOnExit;

	/** Path to your SpatialOS snapshot. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot path"))
	FDirectoryPath SpatialOSSnapshotPath;

	/** Name of your SpatialOS snapshot file. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "Snapshot file name"))
	FString SpatialOSSnapshotFile;

	/** Interop codegen output path */
	UPROPERTY(EditAnywhere, config, Category = "Interop codegen", meta = (ConfigRestartRequired = false, DisplayName = "Output path for the interop codegeneration."))
	FDirectoryPath InteropCodegenOutputFolder;

	/** Generated schema output path */
	UPROPERTY(EditAnywhere, config, Category = "Interop codegen", meta = (ConfigRestartRequired = false, DisplayName = "Output path for the generated schemas."))
	FDirectoryPath GeneratedSchemaOutputFolder;

	UPROPERTY(EditAnywhere, config, Category = "Interop codegen", meta = (ConfigRestartRequired = false, DisplayName = "Singleton classes"))
	TArray<UClass*> SingletonClasses;

	UPROPERTY(EditAnywhere, config, Category = "Interop codegen", meta = (ConfigRestartRequired = false, DisplayName = "Classes to generate typebindings for"))
	TArray<FInteropTypebindingInfo> InteropCodegenClasses;

	UFUNCTION()
	FString ToString();
};

