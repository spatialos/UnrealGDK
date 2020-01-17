// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/AbstractVirtualWorkerTranslator.h"
#include "SpatialCommonTypes.h"

class USpatialVirtualWorkerTranslatorMock : public AbstractVirtualWorkerTranslator
{
public:

	void Init(VirtualWorkerId inReturnVirtualWorkerId);

	virtual VirtualWorkerId GetLocalVirtualWorkerId() const override;

private:
	VirtualWorkerId ReturnVirtualWorkerId;
}; 
