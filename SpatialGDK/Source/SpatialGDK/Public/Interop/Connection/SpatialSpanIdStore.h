// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Misc/DateTime.h"
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

	bool DropSpanIds(const EntityComponentId& Id);
	bool DropSpanId(const EntityComponentId& Id, const uint32 FieldId);
	void DropOldSpanIds();

	worker::c::Trace_SpanId GetSpanId(const EntityComponentId& Id, const uint32 FieldId);
	bool HasSpanIds(const EntityComponentId& Id) const { return EntityComponentFieldSpanIds.Contains(Id); }

private:
	// Private Classes

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

	// Private Members

	const float DropFrequency = 5.0f;
	const float MinSpanIdLifetime = 10.0f;
	const int32 MaxSpanIdsToDrop = 5000;
	FDateTime NextClearTime;

	using FieldIdMap = TMap<uint32, EntityComponentFieldIdUpdateSpanId>;
	TMap<EntityComponentId, FieldIdMap> EntityComponentFieldSpanIds;

	// Private Functions

	void UpdateNextClearTime();
	void AddSpanId(const EntityComponentId& Id, const uint32 FieldId, worker::c::Trace_SpanId SpanId);
	bool DropSpanIdInternal(FieldIdMap* SpanIdMap, const EntityComponentId& Id, const uint32 FieldId);
};
} // namespace SpatialGDK
