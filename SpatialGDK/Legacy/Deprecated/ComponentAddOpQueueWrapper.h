// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialOSWorkerTypes.h"
#include "UObject/NoExportTypes.h"
#include "ComponentUpdateOpWrapperBase.generated.h"

UCLASS(abstract)
class SPATIALOS_API UComponentUpdateOpWrapperBase : public UObject
{
  GENERATED_BODY()
public:
  UComponentUpdateOpWrapperBase()
  {
  }

  ::worker::ComponentId ComponentId;
  ::worker::EntityId EntityId;
};
