// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Engine/EngineTypes.h"

#include "SpatialGDKEditorToolbarSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings, defaultconfig)
class USpatialGDKEditorToolbarSettings : public UObject
{
	GENERATED_BODY()

public:
	USpatialGDKEditorToolbarSettings(const FObjectInitializer& ObjectInitializer);

	/** Root folder of your SpatialOS project */
	UPROPERTY(EditAnywhere, config, Category = SpatialGDKEditor, meta = (ConfigRestartRequired = false))
	FDirectoryPath ProjectRootFolder;

	/** Launch configuration file used for `spatial local launch` */
	UPROPERTY(EditAnywhere, config, Category = SpatialGDKEditor, meta = (ConfigRestartRequired = false))
	FString SpatialGDKLaunchArgument;

	/** Stop spatial.exe when shutting down editor. */
	UPROPERTY(EditAnywhere, config, Category = SpatialGDKEditor, meta = (ConfigRestartRequired = false))
	bool bStopSpatialOnExit;

	UFUNCTION()
	FString ToString();
};
