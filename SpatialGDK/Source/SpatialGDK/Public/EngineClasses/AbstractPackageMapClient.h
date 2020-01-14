// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialConstants.h"
#include "WorkerSDK/improbable/c_worker.h"

class AbstractPackageMapClient
{
public:
	virtual ~AbstractPackageMapClient() {};
	virtual Worker_EntityId GetEntityIdFromObject(const UObject* Object) = 0;
};
