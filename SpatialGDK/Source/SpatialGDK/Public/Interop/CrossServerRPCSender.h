// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Interop/SpatialClassInfoManager.h"
#include "Schema/RPCPayload.h"
#include "SpatialView/CommandRetryHandler.h"
#include "Utils/SpatialMetrics.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCrossServerRPCSender, Log, All);

namespace SpatialGDK
{
class ViewCoordinator;
constexpr FRetryData RETRY_MAX_TIMES = { SpatialConstants::MAX_NUMBER_COMMAND_ATTEMPTS, 0, 0.1f, 5.0f, 0 };

class CrossServerRPCSender
{
public:
	CrossServerRPCSender(ViewCoordinator& Coordinator, USpatialMetrics* SpatialMetrics);
	void SendCommand(const FUnrealObjectRef& InTargetObjectRef, UObject* TargetObject, UFunction* Function, RPCPayload&& InPayload,
					 FRPCInfo Info, const TOptional<Trace_SpanId>& SpanId) const;

private:
	ViewCoordinator& Coordinator;
	USpatialMetrics* SpatialMetrics;
};
} // namespace SpatialGDK
