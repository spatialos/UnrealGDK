// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma once

#include "EntityId.h"
#include "SpatialOSCommandResult.h"
#include "UObject/NoExportTypes.h"

#include "ReserveEntityIdCallbackWrapper.generated.h"

UCLASS()
class UReserveEntityIdCallbackWrapper : public UObject
{
  GENERATED_BODY()

public:
  TFunction<void(const FSpatialOSCommandResult&, FEntityId)> ReserveEntityIdCallback;

  UFUNCTION()
  void ReserveEntityIdCallbackInternal(const FSpatialOSCommandResult& result,
                                       FEntityId reservedEntityId)
  {
    ReserveEntityIdCallback(result, reservedEntityId);
  }
};