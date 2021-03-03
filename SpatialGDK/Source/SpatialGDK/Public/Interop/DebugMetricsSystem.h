// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

class USpatialNetDriver;

namespace SpatialGDK
{
class DebugMetricsSystem
{
public:
	explicit DebugMetricsSystem(USpatialNetDriver& InNetDriver);
	void ProcessOps(const TArray<Worker_Op>& Ops) const;

private:
	USpatialNetDriver& NetDriver;
};
} // namespace SpatialGDK
