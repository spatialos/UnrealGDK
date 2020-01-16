// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"

class SPATIALGDK_API AbstractVirtualWorkerTranslator
{
public:
	virtual ~AbstractVirtualWorkerTranslator() {};
	virtual VirtualWorkerId GetLocalVirtualWorkerId() const PURE_VIRTUAL(UAbstractPackageMapClient::GetEntityIdFromObject, return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;);
};
