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
	/** Root folder of your SpatialOS Unreal GDK project. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false))
	FDirectoryPath ProjectRootFolder;
public:
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
private:
	/** Interop codegen output path */
	UPROPERTY(EditAnywhere, config, Category = "Interop codegen", meta = (ConfigRestartRequired = false, DisplayName = "Output path for the interop codegeneration"))
	FDirectoryPath InteropCodegenOutputFolder;

	/** Generated schema output path */
	UPROPERTY(EditAnywhere, config, Category = "Interop codegen", meta = (ConfigRestartRequired = false, DisplayName = "Output path for the generated schemas"))
	FDirectoryPath GeneratedSchemaOutputFolder;

public:

	UPROPERTY(EditAnywhere, config, Category = "Interop codegen", meta = (ConfigRestartRequired = false, DisplayName = "Singleton classes"))
	TArray<UClass*> SingletonClasses;

	UPROPERTY(EditAnywhere, config, Category = "Interop codegen", meta = (ConfigRestartRequired = false, DisplayName = "Classes to generate typebindings for"))
	TArray<FInteropTypebindingInfo> InteropCodegenClasses;

	UFUNCTION()
	FORCEINLINE FString GetProjectRoot() const
	{
		return ProjectRootFolder.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::GetPath(FPaths::GetProjectFilePath()) + FString(TEXT("/../spatial/")))
			: FPaths::ConvertRelativePathToFull(ProjectRootFolder.Path);
	}

	UFUNCTION()
	FORCEINLINE FString GetInteropCodegenOutputFolder() const
	{
		return InteropCodegenOutputFolder.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::GameSourceDir(), FString::Printf(TEXT("%s/Generated/"), FApp::GetProjectName())))
			: FPaths::ConvertRelativePathToFull(InteropCodegenOutputFolder.Path);
	}

	UFUNCTION()
	FORCEINLINE FString GetGeneratedSchemaOutputFolder() const
	{
		return GeneratedSchemaOutputFolder.Path.IsEmpty()
			? FPaths::ConvertRelativePathToFull(FPaths::Combine(GetProjectRoot(), FString(TEXT("schema/improbable/unreal/generated/"))))
			: FPaths::ConvertRelativePathToFull(GeneratedSchemaOutputFolder.Path);
	}

	UFUNCTION()
	FString ToString();
};

