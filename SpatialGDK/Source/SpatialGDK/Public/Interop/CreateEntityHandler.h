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

class CreateEntityHandler
{
public:
	void AddRequest(Worker_RequestId RequestId, CreateEntityDelegate Handler);
	void ProcessOps(const TArray<Worker_Op>& Ops);

private:
	TMap<Worker_RequestId_Key, CreateEntityDelegate> Handlers;
};
} // namespace SpatialGDK