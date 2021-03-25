// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#include "Utils/SpatialMetrics.h"

class USpatialNetDriver;
class USpatialWorkerConnection;
class USpatialMetrics;

namespace SpatialGDK
{
class SpatialEventTracer;

class DebugMetricsSystem
{
public:
	explicit DebugMetricsSystem(USpatialNetDriver& InNetDriver);
	void ProcessOps(const TArray<Worker_Op>& Ops) const;

private:
	USpatialWorkerConnection& Connection;
	USpatialMetrics& SpatialMetrics;
	SpatialEventTracer* EventTracer;
};
} // namespace SpatialGDK
