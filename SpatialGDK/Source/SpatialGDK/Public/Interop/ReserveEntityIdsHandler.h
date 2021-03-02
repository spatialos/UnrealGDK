// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Interop/SpatialOSDispatcherInterface.h"
#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
class FSubView;

class ReserveEntityHandler
{
public:
	void Advance(const FSubView& SubView);
	void AddRequest(Worker_RequestId RequestId, ReserveEntityIDsDelegate Handler);

private:
	TMap<Worker_RequestId_Key, ReserveEntityIDsDelegate> Handlers;
};
} // namespace SpatialGDK