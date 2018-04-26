#pragma once

#include "ComponentUpdateOpWrapperBase.generated.h"
#include "SpatialGDKWorkerTypes.h"
#include "UObject/NoExportTypes.h"

UCLASS(abstract)
class SPATIALGDK_API UComponentUpdateOpWrapperBase : public UObject {
  GENERATED_BODY()
public:
  UComponentUpdateOpWrapperBase() {}

  ::worker::ComponentId ComponentId;
  ::worker::EntityId EntityId;
};