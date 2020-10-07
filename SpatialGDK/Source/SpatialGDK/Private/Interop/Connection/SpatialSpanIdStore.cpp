// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialSpanIdStore.h"

#include "SpatialConstants.h"
#include <WorkerSDK/improbable/c_trace.h>

DEFINE_LOG_CATEGORY(LogSpatialSpanIdStore);

namespace SpatialGDK
{
void SpatialSpanIdStore::ComponentAdd(const Worker_Op& Op)
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

bool SpatialSpanIdStore::ComponentRemove(const Worker_Op& Op)
{
	const Worker_RemoveComponentOp& RemoveComponentOp = Op.op.remove_component;
	EntityComponentId Id(RemoveComponentOp.entity_id, RemoveComponentOp.component_id);
	return DropSpanIds(Id);
}

TArray<SpatialSpanIdStore::FieldSpanIdUpdate> SpatialSpanIdStore::ComponentUpdate(const Worker_Op& Op)
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

void SpatialSpanIdStore::AddSpanId(const EntityComponentId& Id, const uint32 FieldId, Trace_SpanId SpanId)
{
	FieldIdMap& SpanIdMap = EntityComponentFieldSpanIds.FindOrAdd(Id);
	EntityComponentFieldIdSpanIdUpdate& SpawnIdRef = SpanIdMap.FindOrAdd(FieldId);
	SpawnIdRef.SpanId = SpanId;
	SpawnIdRef.UpdateTime = FDateTime::Now();
}

bool SpatialSpanIdStore::DropSpanId(const EntityComponentId& Id, const uint32 FieldId)
{
	int32 NumRemoved = DropSpanIdInternal(EntityComponentFieldSpanIds.Find(Id), Id, FieldId);
	return NumRemoved > 0;
}

bool SpatialSpanIdStore::DropSpanIds(const EntityComponentId& Id)
{
	int32 NumRemoved = EntityComponentFieldSpanIds.Remove(Id) > 0;
	EntityComponentFieldSpanIds.Compact();
	return NumRemoved > 0;
}

bool SpatialSpanIdStore::DropSpanIdInternal(FieldIdMap* SpanIdMap, const EntityComponentId& Id, const uint32 FieldId)
{
	if (SpanIdMap == nullptr)
	{
		return false;
	}

	bool bDropped = SpanIdMap->Remove(FieldId) > 0;
	if (bDropped)
	{
		CompactCounter++;
		if (CompactCounter >= CompactFrequency)
		{
			CompactCounter = 0;
			EntityComponentFieldSpanIds.Compact();
		}

		if (SpanIdMap->Num() == 0)
		{
			EntityComponentFieldSpanIds.Remove(Id);
		}
	}

	return bDropped;
}

bool SpatialSpanIdStore::GetSpanId(const EntityComponentId& Id, const uint32 FieldId, Trace_SpanId& OutSpanId, bool bRemove /*= true*/)
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
		DropSpanIdInternal(SpanIdMap, Id, FieldId);
	}

	return true;
}

bool SpatialSpanIdStore::GetMostRecentSpanId(const EntityComponentId& Id, Trace_SpanId& OutSpanId, bool bRemove /*= true*/)
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
		DropSpanIdInternal(SpanIdMap, Id, FieldId);
	}

	return true;
}
} // namespace SpatialGDK
