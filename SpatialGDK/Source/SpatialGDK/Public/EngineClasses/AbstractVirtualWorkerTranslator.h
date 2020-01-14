// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"

class AbstractVirtualWorkerTranslator 
{
public:
	virtual ~AbstractVirtualWorkerTranslator() {};
	virtual VirtualWorkerId GetLocalVirtualWorkerId() const = 0;
};
