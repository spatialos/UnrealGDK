// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialSpanIdCache.h"

#include "SpatialConstants.h"
#include <WorkerSDK/improbable/c_trace.h>

DEFINE_LOG_CATEGORY(LogSpatialSpanIdStore);

using namespace SpatialGDK;
using namespace worker::c;

void SpatialSpanIdCache::ComponentAdd(const Worker_Op& Op)
{
	const Worker_AddComponentOp& AddComponentOp = Op.op.add_component;
	EntityComponentId Id(AddComponentOp.entity_id, AddComponentOp.data.component_id);
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(AddComponentOp.data.schema_type);

	TArray<uint32> UpdatedFieldIds;
	UpdatedFieldIds.SetNumUninitialized(Schema_GetUniqueFieldIdCount(ComponentObject));
	Schema_GetUniqueFieldIds(ComponentObject, UpdatedFieldIds.GetData());

	for (uint32 FieldId : UpdatedFieldIds)
	{
		AddSpanId(Id, FieldId, Op.span_id);
	}
}

bool SpatialSpanIdCache::ComponentRemove(const Worker_Op& Op)
{
	const Worker_RemoveComponentOp& RemoveComponentOp = Op.op.remove_component;
	EntityComponentId Id(RemoveComponentOp.entity_id, RemoveComponentOp.component_id);
	return DropSpanIds(Id);
}

TArray<SpatialSpanIdCache::FieldSpanIdUpdate> SpatialSpanIdCache::ComponentUpdate(const Worker_Op& Op)
{
	const Worker_ComponentUpdateOp& ComponentUpdateOp = Op.op.component_update;
	EntityComponentId Id(ComponentUpdateOp.entity_id, ComponentUpdateOp.update.component_id);
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdateOp.update.schema_type);

	TArray<uint32> UpdatedFieldIds;
	UpdatedFieldIds.SetNumUninitialized(Schema_GetUniqueFieldIdCount(ComponentObject));
	Schema_GetUniqueFieldIds(ComponentObject, UpdatedFieldIds.GetData());

	TArray<FieldSpanIdUpdate> FieldCollisions;
	for (uint32 FieldId : UpdatedFieldIds)
	{
		Trace_SpanId ExistingSpanId;
		if (GetSpanId(Id, FieldId, ExistingSpanId))
		{
			FieldSpanIdUpdate Update;
			Update.FieldId = FieldId;
			Update.NewSpanId = Op.span_id;
			Update.OldSpanId = ExistingSpanId;
			FieldCollisions.Add(Update);
		}

		AddSpanId(Id, FieldId, Op.span_id);
	}

	return FieldCollisions;
}

void SpatialSpanIdCache::AddSpanId(const EntityComponentId& Id, const uint32 FieldId, Trace_SpanId SpanId)
{
	FieldIdMap& SpanIdMap = EntityComponentFieldSpanIds.FindOrAdd(Id);
	EntityComponentFieldIdSpanIdUpdate& SpawnIdRef = SpanIdMap.FindOrAdd(FieldId);
	SpawnIdRef.SpanId = SpanId;
	SpawnIdRef.UpdateTime = FDateTime::Now();
}

bool SpatialSpanIdCache::DropSpanId(const EntityComponentId& Id, const uint32 FieldId)
{
	return DropSpanIdInternal(EntityComponentFieldSpanIds.Find(Id), Id, FieldId);
}

bool SpatialSpanIdCache::DropSpanIds(const EntityComponentId& Id)
{
	return EntityComponentFieldSpanIds.Remove(Id) > 0;
}

bool SpatialSpanIdCache::DropSpanIdInternal(FieldIdMap* SpanIdMap, const EntityComponentId& Id, const uint32 FieldId)
{
	if (SpanIdMap == nullptr)
	{
		return false;
	}

	bool bDropped = SpanIdMap->Remove(FieldId) > 0;
	if (bDropped && SpanIdMap->Num() == 0)
	{
		EntityComponentFieldSpanIds.Remove(Id);
	}

	return bDropped;
}

bool SpatialSpanIdCache::GetSpanId(const EntityComponentId& Id, const uint32 FieldId, Trace_SpanId& OutSpanId, bool bRemove /*= true*/)
{
	FieldIdMap* SpanIdMap = EntityComponentFieldSpanIds.Find(Id);
	if (SpanIdMap == nullptr)
	{
		return false;
	}

	EntityComponentFieldIdSpanIdUpdate* UpdateSpanId = SpanIdMap->Find(FieldId);
	if (UpdateSpanId == nullptr)
	{
		return false;
	}

	OutSpanId = UpdateSpanId->SpanId;

	if (bRemove)
	{
		SpanIdMap->Remove(FieldId);
	}

	return true;
}

bool SpatialSpanIdCache::GetMostRecentSpanId(const EntityComponentId& Id, Trace_SpanId& OutSpanId, bool bRemove /*= true*/)
{
	FieldIdMap* SpanIdMap = EntityComponentFieldSpanIds.Find(Id);
	if (SpanIdMap == nullptr)
	{
		return false;
	}

	if (SpanIdMap->Num() == 0)
	{
		return false;
	}

	uint32 FieldId = 0;
	EntityComponentFieldIdSpanIdUpdate MostRecent;
	for (const auto& Pair : *SpanIdMap)
	{
		const EntityComponentFieldIdSpanIdUpdate& Data = Pair.Value;
		if (Data.UpdateTime > MostRecent.UpdateTime || Trace_SpanId_Equal(MostRecent.SpanId, Trace_SpanId()))
		{
			FieldId = Pair.Key;
			MostRecent = Data;
		}
	}

	OutSpanId = MostRecent.SpanId;

	if (bRemove)
	{
		SpanIdMap->Remove(FieldId);
	}

	return true;
}
