// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKWorkerTypes.h"
#include "AddComponentOpWrapperBase.generated.h"

UCLASS(abstract)
class SPATIALGDK_API UAddComponentOpWrapperBase : public UObject
{
  GENERATED_BODY()
public:
  UAddComponentOpWrapperBase()
  {
  }

  ::worker::ComponentId ComponentId;
  ::worker::EntityId EntityId;
};