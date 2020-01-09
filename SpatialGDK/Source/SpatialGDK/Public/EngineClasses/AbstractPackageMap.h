#pragma once

#include "SpatialConstants.h"
#include "WorkerSDK/improbable/c_worker.h"

#include "UObject/Object.h"

class AbstractPackageMap 
{
public:
	virtual Worker_EntityId GetEntityIdFromObject(const UObject* Object) = 0;
};
