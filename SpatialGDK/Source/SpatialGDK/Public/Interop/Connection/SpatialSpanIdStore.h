// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Misc/DateTime.h"
#include "SpatialCommonTypes.h"
#include "SpatialConstants.h"
#include "SpatialView/EntityComponentId.h"

#include <WorkerSDK/improbable/c_worker.h>

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialSpanIdStore, Log, All);

namespace SpatialGDK
{
class SpatialSpanIdStore
{
public:

	SpatialSpanIdStore();

	void ComponentAdd(const Worker_Op& Op);
	bool ComponentRemove(const Worker_Op& Op);
	void ComponentUpdate(const Worker_Op& Op);

	worker::c::Trace_SpanId GetEntityComponentFieldSpanId(const EntityComponentId& Id, const uint32 FieldId);
	bool ClearEntityComponentSpanIds(const EntityComponentId& Id);
	bool RemoveEntityComponentFieldSpanId(const EntityComponentId& Id, const uint32 FieldId);

	bool HasSpanIdForEntityComponent(const EntityComponentId& Id) const { return EntityComponentFieldSpanIds.Contains(Id); }

	void DropOldUpdates();

private:

	float ClearFrequency = 10.0f;
	float MinUpdateLifetime = 10.0f;
	int32 MaxUpdateDrops = 5000;
	FDateTime NextClearTime;

	struct EntityComponentFieldId
	{
		EntityComponentId EntityComponentId;
		uint32 FieldId;
	};

	struct EntityComponentFieldIdUpdateSpanId
	{
		worker::c::Trace_SpanId SpanId;
		FDateTime UpdateTime;
	};

	using FieldIdMap = TMap<uint32, EntityComponentFieldIdUpdateSpanId>;
	TMap<EntityComponentId, FieldIdMap> EntityComponentFieldSpanIds;

	void UpdateNextClearTime();

	void ProcessRPCComponentUpdate(const EntityComponentId& Id, Schema_Object* SchemaObject, ERPCType RPCType, worker::c::Trace_SpanId SpanId);
	void AddEntityComponentFieldSpanId(const EntityComponentId& Id, const uint32 FieldId, worker::c::Trace_SpanId SpanId);

	bool RemoveEntityComponentFieldInternal(FieldIdMap* SpanIdMap, const EntityComponentId& Id, const uint32 FieldId);

	static bool IsComponentIdRPCEndpoint(const Worker_ComponentId ComponentId);
};
} // namespace SpatialGDK
