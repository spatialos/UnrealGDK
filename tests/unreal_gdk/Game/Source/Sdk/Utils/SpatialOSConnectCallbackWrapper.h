// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "SpatialOSCommandResult.h"
#include "UObject/NoExportTypes.h"

#include "SpatialOSConnectCallbackWrapper.generated.h"

UCLASS()
class USpatialOSConnectCallbackWrapper : public UObject
{
  GENERATED_BODY()

public:
  bool bIsConnection;

  TFunction<void()> ConnectedCallback;

  UFUNCTION()
  void ConnectedCallbackInternal()
  {
    bIsConnection = true;
    ConnectedCallback();
  }
};