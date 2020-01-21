// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialVirtualWorkerTranslatorMock.h"

USpatialVirtualWorkerTranslatorMock::USpatialVirtualWorkerTranslatorMock(VirtualWorkerId VirtWorkerId)
	: VirtWorkerId( VirtWorkerId ) {}

VirtualWorkerId USpatialVirtualWorkerTranslatorMock::GetLocalVirtualWorkerId() const
{
	return VirtWorkerId;
}
