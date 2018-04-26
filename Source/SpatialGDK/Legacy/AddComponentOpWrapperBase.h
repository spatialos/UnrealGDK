#pragma once

#include "AddComponentOpWrapperBase.generated.h"
#include "SpatialGDKWorkerTypes.h"
#include "UObject/NoExportTypes.h"

UCLASS(abstract)
class SPATIALGDK_API UAddComponentOpWrapperBase : public UObject {
  GENERATED_BODY()
public:
  UAddComponentOpWrapperBase() {}

  ::worker::ComponentId ComponentId;
  ::worker::EntityId EntityId;
};