// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once
#include "SpatialOSWorkerConfigurationData.h"

#include "SpatialGDKSettings.generated.h"

USTRUCT()
struct FSpatialOSWorkerOverrideSettings
{
  GENERATED_BODY()

  FSpatialOSWorkerOverrideSettings();

  /** SpatialOS worker configuration */
  UPROPERTY(EditAnywhere, config, Category = WorkerOverride)
  FSpatialGDKWorkerConfigurationData WorkerConfigurationData;

  /** Should viewport rendering be disabled? */
  UPROPERTY(EditAnywhere, config, Category = WorkerOverride)
  bool bDisableRendering;
};

UCLASS(config = Editor, defaultconfig)
class USpatialGDKSettings : public UObject
{
  GENERATED_BODY()

public:
  USpatialGDKSettings(const FObjectInitializer& ObjectInitializer);

  /** Should user worker configurations be used? */
  UPROPERTY(EditAnywhere, config, Category = Custom, meta = (ConfigRestartRequired = false))
  bool bUseUserWorkerConfigurations;

  /** Array of worker configurations */
  // clang-format off
  UPROPERTY(EditAnywhere, config, Category = Custom, meta = (ConfigRestartRequired = false, EditCondition = bUseUserWorkerConfigurations))
  // clang-format on
  TArray<FSpatialOSWorkerOverrideSettings> WorkerConfigurations;
};
