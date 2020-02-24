// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialStaticComponentView.h"
#include "SpatialCommonTypes.h"
#include "WorkerSDK/improbable/c_worker.h"

#include "SpatialStaticComponentViewMock.generated.h"

UCLASS()
class USpatialStaticComponentViewMock : public USpatialStaticComponentView
{
	GENERATED_BODY()
public:
	void Init(Worker_EntityId EntityId, Worker_Authority InAuthority, VirtualWorkerId VirtWorkerId);
}; 
