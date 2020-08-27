// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"
#include "SpatialView/EntityComponentId.h"

#include <WorkerSDK/improbable/c_worker.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialSpanIdStore, Log, All);

namespace SpatialGDK
{
class SpatialSpanIdStore
{
public:
	worker::c::Trace_SpanId* GetEntityComponentSpanId(const EntityComponentId& Id);
	bool ComponentAdd(const EntityComponentId Id, const worker::c::Trace_SpanId SpanId);
	bool ComponentRemove(const EntityComponentId Id, const worker::c::Trace_SpanId SpanId);
	void ComponentUpdate(const EntityComponentId Id, const worker::c::Trace_SpanId SpanId);

	worker::c::Trace_SpanId GetNextRPCSpanID();
	void RemoveRPCSpanIds(int32 NumToRemove);

	void Clear();

private:
	TMap<EntityComponentId, worker::c::Trace_SpanId> SpanStore;
	TArray<worker::c::Trace_SpanId> RPCSpanIds;

	static bool IsComponentIdRPCEndpoint(const Worker_ComponentId ComponentId);
};
} // namespace SpatialGDK
