// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "SpatialGDKEditorSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings, defaultconfig)
class USpatialGDKEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKEditorSettings(const FObjectInitializer& ObjectInitializer);

private:
	/** Path to the directory containing the SpatialOS-related files. */
	UPROPERTY(EditAnywhere, config, Category = "Configuration", meta = (ConfigRestartRequired = false, DisplayName = "SpatialOS directory"))
		FDirectoryPath SpatialOSDirectory;

public:
	/** Generate schema for all classes supported by the GDK */
	UPROPERTY(EditAnywhere, config, Category = "Schema Generation", meta = (ConfigRestartRequired = false, DisplayName = "Generate schema for all supported classes"))
	bool bGenerateSchemaForAllSupportedClasses;

private:
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
};
