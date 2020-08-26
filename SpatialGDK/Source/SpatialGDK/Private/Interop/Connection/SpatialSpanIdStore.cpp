// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialSpanIdStore.h"

#include <WorkerSDK/improbable/c_trace.h>

DEFINE_LOG_CATEGORY(LogSpatialSpanIdStore);

using namespace SpatialGDK;
using namespace worker::c;

Trace_SpanId* SpatialSpanIdStore::GetEntityComponentSpanId(const EntityComponentId& Id)
{
	return SpanStore.Find(Id);
}

bool SpatialSpanIdStore::ComponentAdd(const EntityComponentId Id, const Trace_SpanId SpanId)
{
	if (IsComponentIdRPCEndpoint(Id.EntityId))
	{
		RPCSpanIds.Add(SpanId);
		return;
	}

	if (!SpanStore.Contains(Id))
	{
		SpanStore.Add(Id, SpanId);
		return true;
	}
	return false;
}

bool SpatialSpanIdStore::ComponentRemove(const EntityComponentId Id, const Trace_SpanId SpanId)
{
	if (IsComponentIdRPCEndpoint(Id.EntityId))
	{
		RPCSpanIds.Empty();
		return;
	}

	if (SpanStore.Contains(Id))
	{
		SpanStore.Remove(Id);
		return true;
	}
	return false;
}

void SpatialSpanIdStore::ComponentUpdate(const EntityComponentId Id, const Trace_SpanId SpanId)
{
	if (IsComponentIdRPCEndpoint(Id.EntityId))
	{
		RPCSpanIds.Add(SpanId);
		return;
	}

	Trace_SpanId* StoredSpanId = SpanStore.Find(Id);
	if (StoredSpanId != nullptr)
	{
		*StoredSpanId = SpanId;
	}

	SpanStore.Add(Id, SpanId);
}

worker::c::Trace_SpanId SpatialSpanIdStore::GetNextRPCSpanID()
{
	const int32 NumRPCSpanIds = RPCSpanIds.Num();
	if (NumRPCSpanIds == 0)
	{
		return worker::c::Trace_SpanId();
	}
	return RPCSpanIds.Pop();
}

void SpatialSpanIdStore::Clear()
{
	SpanStore.Empty();
}

bool SpatialSpanIdStore::IsComponentIdRPCEndpoint(const Worker_ComponentId ComponentId)
{
	return Id.EntityId == SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID || Id.EntityId == SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID
}
