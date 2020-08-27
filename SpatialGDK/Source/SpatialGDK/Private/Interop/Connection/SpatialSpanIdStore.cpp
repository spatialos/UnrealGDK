// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialSpanIdStore.h"

#include "SpatialConstants.h"
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
	if (IsComponentIdRPCEndpoint(Id.ComponentId))
	{
		RPCSpanIds.Add(SpanId);
		return true;
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
	if (IsComponentIdRPCEndpoint(Id.ComponentId))
	{
		RPCSpanIds.Empty();
		return true;
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
	if (IsComponentIdRPCEndpoint(Id.ComponentId))
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
	UE_LOG(LogSpatialSpanIdStore, Log, TEXT("RPCSpanId queue size - %d"), NumRPCSpanIds);

	if (NumRPCSpanIds == 0)
	{
		return worker::c::Trace_SpanId();
	}
	return RPCSpanIds.Pop();
}

void SpatialSpanIdStore::RemoveRPCSpanIds(int32 NumToRemove)
{
	const int32 FirstIndex = RPCSpanIds.Num() - 1;
	const int32 IndexToStop = FMath::Max(0, FirstIndex - NumToRemove);

	for (int i = FirstIndex; i > IndexToStop; --i)
	{
		RPCSpanIds.Pop();
	}
}

void SpatialSpanIdStore::Clear()
{
	SpanStore.Empty();
}

bool SpatialSpanIdStore::IsComponentIdRPCEndpoint(const Worker_ComponentId ComponentId)
{
	return ComponentId == SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID || ComponentId == SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID;
}
