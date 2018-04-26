#pragma once

#include "SpatialGDKWorkerTypes.h"
#include "UObject/NoExportTypes.h"
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