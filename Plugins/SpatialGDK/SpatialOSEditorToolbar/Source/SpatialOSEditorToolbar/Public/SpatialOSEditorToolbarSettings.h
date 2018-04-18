// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "Engine/EngineTypes.h"

#include "SpatialOSEditorToolbarSettings.generated.h"

UCLASS(config = EditorPerProjectUserSettings, defaultconfig)
class USpatialOSEditorToolbarSettings : public UObject
{
  GENERATED_BODY()

public:
  USpatialOSEditorToolbarSettings(const FObjectInitializer& ObjectInitializer);

  /** Root folder of your SpatialOS project */
  UPROPERTY(EditAnywhere, config, Category = SpatialOSEditor,
            meta = (ConfigRestartRequired = false))
  FDirectoryPath ProjectRootFolder;

  /** Launch configuration file used for `spatial local launch` */
  UPROPERTY(EditAnywhere, config, Category = SpatialOSEditor,
            meta = (ConfigRestartRequired = false))
  FString SpatialOSLaunchArgument;

  /** Stop spatial.exe when shutting down editor. */
  UPROPERTY(EditAnywhere, config, Category = SpatialOSEditor,
            meta = (ConfigRestartRequired = false))
  bool bStopSpatialOnExit;

  UFUNCTION()
  FString ToString();
};
