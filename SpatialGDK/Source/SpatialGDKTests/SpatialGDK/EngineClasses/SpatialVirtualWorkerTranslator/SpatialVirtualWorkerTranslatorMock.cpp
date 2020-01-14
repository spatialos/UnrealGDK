// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialVirtualWorkerTranslatorMock.h"

void USpatialVirtualWorkerTranslatorMock::Init(VirtualWorkerId inVirtualWorkerId)
{
	ReturnVirtualWorkerId = inVirtualWorkerId;
}

VirtualWorkerId USpatialVirtualWorkerTranslatorMock::GetLocalVirtualWorkerId() const
{
	return ReturnVirtualWorkerId;
}
