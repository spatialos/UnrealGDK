// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Misc/DateTime.h"
#include "SpatialView/EntityComponentId.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialSpanIdStore, Log, All);

namespace SpatialGDK
{
class SpatialSpanIdStore
{
public:
	SpatialSpanIdStore();

	struct FieldSpanIdUpdate
	{
		uint32 FieldId;
		worker::c::Trace_SpanId NewSpanId;
		worker::c::Trace_SpanId OldSpanId;
	};

	void ComponentAdd(const Worker_Op& Op);
	bool ComponentRemove(const Worker_Op& Op);

	// Returns a list of the field ids that already existed in the store
	TArray<FieldSpanIdUpdate> ComponentUpdate(const Worker_Op& Op);

	void WriteSpanId(const EntityComponentId& Id, const uint32 FieldId, worker::c::Trace_SpanId SpanId);

	bool DropSpanIds(const EntityComponentId& Id);
	bool DropSpanId(const EntityComponentId& Id, const uint32 FieldId);
	void DropOldSpanIds();

	worker::c::Trace_SpanId GetSpanId(const EntityComponentId& Id, const uint32 FieldId);
	worker::c::Trace_SpanId GetMostRecentSpanId(const EntityComponentId& Id);

private:
	// Private Classes

	struct EntityComponentFieldId
	{
		EntityComponentId EntityComponentId;
		uint32 FieldId;
	};

	struct EntityComponentFieldIdUpdateSpanId
	{
		worker::c::Trace_SpanId SpanId = worker::c::Trace_SpanId();
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
	bool DropSpanIdInternal(FieldIdMap* SpanIdMap, const EntityComponentId& Id, const uint32 FieldId);
};
} // namespace SpatialGDK
