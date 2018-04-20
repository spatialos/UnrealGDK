// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "AddComponentOpWrapperBase.h"
#include "CoreMinimal.h"
#include "EngineMinimal.h"
#include "ComponentAddOpQueueWrapper.generated.h"

USTRUCT()
struct FComponentAddOpQueueWrapper
{
  GENERATED_USTRUCT_BODY()

public:
  UPROPERTY()
  TArray<UAddComponentOpWrapperBase*> Underlying;

  FComponentAddOpQueueWrapper()
  {
  }
};
