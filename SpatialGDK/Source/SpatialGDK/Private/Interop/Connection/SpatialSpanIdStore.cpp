// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialSpanIdStore.h"

#include "SpatialConstants.h"
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

	TArray<uint32> UpdatedFieldIds;
	UpdatedFieldIds.SetNumUninitialized(Schema_GetUniqueFieldIdCount(ComponentObject));
	Schema_GetUniqueFieldIds(ComponentObject, UpdatedFieldIds.GetData());

	for (uint32 FieldId : UpdatedFieldIds)
	{
		WriteSpanId(Id, FieldId, Op.span_id);
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
		FieldIdMap* SpanIdMap = EntityComponentFieldSpanIds.Find(Id);
		if (SpanIdMap != nullptr)
		{
			EntityComponentFieldIdUpdateSpanId* SpawnIdRef = SpanIdMap->Find(FieldId);
			if (SpawnIdRef != nullptr)
			{
				FieldSpanIdUpdate Update;
				Update.FieldId = FieldId;
				Update.NewSpanId = Op.span_id;
				Update.OldSpanId = SpawnIdRef->SpanId;
				FieldCollisions.Add(Update);
			}
		}

		WriteSpanId(Id, FieldId, Op.span_id);
	}

	return FieldCollisions;
}

void SpatialSpanIdStore::WriteSpanId(const EntityComponentId& Id, const uint32 FieldId, worker::c::Trace_SpanId SpanId)
{
	FieldIdMap& SpanIdMap = EntityComponentFieldSpanIds.FindOrAdd(Id);
	EntityComponentFieldIdUpdateSpanId& SpawnIdRef = SpanIdMap.FindOrAdd(FieldId);
	SpawnIdRef.SpanId = SpanId;
	SpawnIdRef.UpdateTime = FDateTime::Now();
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
	return EntityComponentFieldSpanIds.Remove(Id) > 0;
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

	bool bDropped = SpanIdMap->Remove(FieldId) > 0;
	if (bDropped && SpanIdMap->Num() == 0)
	{
		EntityComponentFieldSpanIds.Remove(Id);
	}

	return bDropped;
}

void SpatialSpanIdStore::DropOldSpanIds()
{
	if (NextClearTime < FDateTime::Now())
	{
		UpdateNextClearTime();

		FDateTime DropDateTime = FDateTime::Now() - FTimespan::FromSeconds(MinSpanIdLifetime);

		TArray<EntityComponentFieldId> EntityComponentFieldIdsToDrop;
		int32 NumDropped = 0;
		bool bShouldBreak = false;

		for (const auto& Pair : EntityComponentFieldSpanIds)
		{
			const EntityComponentId& Id = Pair.Key;
			const FieldIdMap& Map = Pair.Value;

			for (const auto& UpdateSpanIdPair : Map)
			{
				uint32 FieldId = UpdateSpanIdPair.Key;
				const EntityComponentFieldIdUpdateSpanId& UpdateSpanId = UpdateSpanIdPair.Value;

				if (UpdateSpanId.UpdateTime < DropDateTime)
				{
					EntityComponentFieldIdsToDrop.Add({ Id, FieldId });
					NumDropped++;
					if (NumDropped >= MaxSpanIdsToDrop)
					{
						UE_LOG(LogSpatialSpanIdStore, Log,
							   TEXT("Too many SpanIds to drop in a single call. Will attempt to drop the rest later."));
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

		for (const EntityComponentFieldId& Id : EntityComponentFieldIdsToDrop)
		{
			DropSpanId(Id.EntityComponentId, Id.FieldId);
		}

		UE_LOG(LogSpatialSpanIdStore, Verbose, TEXT("Periodic SpanId drop dropped %d SpanIds."), EntityComponentFieldIdsToDrop.Num());

		EntityComponentFieldSpanIds.Compact();
	}
}

void SpatialSpanIdStore::UpdateNextClearTime()
{
	NextClearTime = FDateTime::Now() + FTimespan::FromSeconds(DropFrequency);
}
