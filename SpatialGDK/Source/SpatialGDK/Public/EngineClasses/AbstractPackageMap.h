#pragma once

#include "SpatialConstants.h"
#include "WorkerSDK/improbable/c_worker.h"

class AbstractPackageMap 
{
public:
	virtual Worker_EntityId GetEntityIdFromObject(const UObject* Object) { return SpatialConstants::INVALID_ENTITY_ID; };
};
