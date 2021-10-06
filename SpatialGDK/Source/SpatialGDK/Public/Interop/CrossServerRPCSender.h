// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/RPCPayload.h"
#include "SpatialView/ViewCoordinator.h"
#include "Utils/SpatialMetrics.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCrossServerRPCSender, Log, All);

namespace SpatialGDK
{
class SpatialEventTracer;
class CrossServerRPCSender
{
public:
	CrossServerRPCSender(ViewCoordinator& Coordinator, USpatialMetrics* SpatialMetrics, SpatialEventTracer* EventTracer);
	void SendCommand(const FUnrealObjectRef InTargetObjectRef, UObject* TargetObject, UFunction* Function, RPCPayload&& InPayload,
					 FRPCInfo Info) const;

private:
	ViewCoordinator* Coordinator;
	USpatialMetrics* SpatialMetrics;
	SpatialEventTracer* EventTracer;
};
} // namespace SpatialGDK
