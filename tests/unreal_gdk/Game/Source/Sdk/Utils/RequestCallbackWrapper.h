// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "EntityId.h"
#include "SpatialOSCommandResult.h"
#include "UObject/NoExportTypes.h"

#include "RequestCallbackWrapper.generated.h"

UCLASS()
class URequestCallbackWrapper : public UObject
{
  GENERATED_BODY()

public:
  TFunction<void(const FSpatialOSCommandResult&, UTestType1*)> RequestTestdata1Callback;

  UFUNCTION()
  void RequestTestdata1CallbackInternal(const FSpatialOSCommandResult& result, UTestType1* response)
  {
    RequestTestdata1Callback(result, response);
  }
};