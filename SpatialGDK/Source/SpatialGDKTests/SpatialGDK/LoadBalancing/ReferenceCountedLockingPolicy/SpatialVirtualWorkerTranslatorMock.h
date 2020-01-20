// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/AbstractVirtualWorkerTranslator.h"
#include "SpatialCommonTypes.h"

class USpatialVirtualWorkerTranslatorMock : public AbstractVirtualWorkerTranslator
{
public:
	USpatialVirtualWorkerTranslatorMock(VirtualWorkerId VirtWorkerId);

	virtual VirtualWorkerId GetLocalVirtualWorkerId() const override;

private:
	VirtualWorkerId VirtWorkerId;
}; 
