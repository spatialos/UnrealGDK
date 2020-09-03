// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialSpanIdStore.h"

#include "Schema/RPCPayload.h"
#include "SpatialConstants.h"
#include "Utils/RPCRingBuffer.h"
#include <WorkerSDK/improbable/c_trace.h>

DEFINE_LOG_CATEGORY(LogSpatialSpanIdStore);

using namespace SpatialGDK;
using namespace worker::c;

SpatialSpanIdStore::SpatialSpanIdStore()
{
	UpdateNextClearTime();
}

void SpatialSpanIdStore::ComponentAdd(const Worker_Op& Op)
{
	const Worker_AddComponentOp& AddComponentOp = Op.op.add_component;
	EntityComponentId Id(AddComponentOp.entity_id, AddComponentOp.data.component_id);
	Schema_Object* ComponentObject = Schema_GetComponentDataFields(AddComponentOp.data.schema_type);

	TArray<uint32> UpdatedIds;
	UpdatedIds.SetNumUninitialized(Schema_GetUniqueFieldIdCount(ComponentObject));
	Schema_GetUniqueFieldIds(ComponentObject, UpdatedIds.GetData());

	for (uint32 FieldId : UpdatedIds)
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

void SpatialSpanIdStore::ComponentUpdate(const Worker_Op& Op)
{
	const Worker_ComponentUpdateOp& ComponentUpdateOp = Op.op.component_update;
	EntityComponentId Id(ComponentUpdateOp.entity_id, ComponentUpdateOp.update.component_id);
	Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdateOp.update.schema_type);

	TArray<uint32> UpdatedIds;
	UpdatedIds.SetNumUninitialized(Schema_GetUniqueFieldIdCount(ComponentObject));
	Schema_GetUniqueFieldIds(ComponentObject, UpdatedIds.GetData());

	for (uint32 FieldId : UpdatedIds)
	{
		AddSpanId(Id, FieldId, Op.span_id);
	}
}

worker::c::Trace_SpanId SpatialSpanIdStore::GetSpanId(const EntityComponentId& Id, const uint32 FieldId)
{
	worker::c::Trace_SpanId ReturnSpanId = worker::c::Trace_SpanId();

	FieldIdMap* SpanIdMap = EntityComponentFieldSpanIds.Find(Id);
	if (SpanIdMap == nullptr)
	{
		return ReturnSpanId;
	}

	EntityComponentFieldIdUpdateSpanId* UpdateSpanId = SpanIdMap->Find(FieldId);
	if (UpdateSpanId == nullptr)
	{
		return ReturnSpanId;
	}

	ReturnSpanId = UpdateSpanId->SpanId;
	DropSpanIdInternal(SpanIdMap, Id, FieldId);

	return ReturnSpanId;
}

bool SpatialSpanIdStore::DropSpanIds(const EntityComponentId& Id)
{
	return EntityComponentFieldSpanIds.Remove(Id);
}

bool SpatialSpanIdStore::DropSpanId(const EntityComponentId& Id, const uint32 FieldId)
{
	return DropSpanIdInternal(EntityComponentFieldSpanIds.Find(Id), Id, FieldId);
}

bool SpatialSpanIdStore::DropSpanIdInternal(FieldIdMap* SpanIdMap, const EntityComponentId& Id, const uint32 FieldId)
{
	if (SpanIdMap == nullptr)
	{
		return false;
	}

	bool bRemoved = SpanIdMap->Remove(FieldId);
	if (bRemoved && SpanIdMap->Num() == 0)
	{
		EntityComponentFieldSpanIds.Remove(Id);
	}

	return bRemoved;
}

void SpatialSpanIdStore::DropOldSpanIds()
{
	if (NextClearTime < FDateTime::Now())
	{
		UpdateNextClearTime();

		FDateTime RemoveDateTime = FDateTime::Now() - FTimespan::FromSeconds(MinUpdateLifetime);

		TArray<EntityComponentFieldId> EntityComponentFieldIdsToRemove;
		int32 NumDropped = 0;
		bool bShouldBreak = false;

		for (auto& Pair : EntityComponentFieldSpanIds)
		{
			const EntityComponentId& Id = Pair.Key;
			FieldIdMap Map = Pair.Value;

			for (auto& UpdateSpanIdPair : Map)
			{
				uint32 FieldId = UpdateSpanIdPair.Key;
				EntityComponentFieldIdUpdateSpanId& UpdateSpanId = UpdateSpanIdPair.Value;
				if (UpdateSpanId.UpdateTime < RemoveDateTime)
				{
					EntityComponentFieldIdsToRemove.Add({ Id, FieldId });
					NumDropped++;
					if (NumDropped >= MaxUpdateDrops)
					{
						bShouldBreak = true;
						break;
					}
				}
			}

			if (bShouldBreak)
			{
				break;
			}
		}

		for (const EntityComponentFieldId& Id : EntityComponentFieldIdsToRemove)
		{
			DropSpanId(Id.EntityComponentId, Id.FieldId);
		}

		EntityComponentFieldSpanIds.Compact();
	}
}

void SpatialSpanIdStore::AddSpanId(const EntityComponentId& Id, const uint32 FieldId, worker::c::Trace_SpanId SpanId)
{
	FieldIdMap* SpanIdMap = EntityComponentFieldSpanIds.Find(Id);

	if (SpanIdMap == nullptr)
	{
		SpanIdMap = &EntityComponentFieldSpanIds.Add(Id);
	}

	EntityComponentFieldIdUpdateSpanId& SpawnIdRef = SpanIdMap->FindOrAdd(FieldId);
	SpawnIdRef.SpanId = SpanId;
	SpawnIdRef.UpdateTime = FDateTime::Now();
}

void SpatialSpanIdStore::UpdateNextClearTime()
{
	NextClearTime = FDateTime::Now() + FTimespan::FromSeconds(ClearFrequency);
}
