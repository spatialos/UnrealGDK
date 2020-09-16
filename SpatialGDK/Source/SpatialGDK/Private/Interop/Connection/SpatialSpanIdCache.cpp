// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialSpanIdCache.h"

#include "SpatialConstants.h"
#include <WorkerSDK/improbable/c_trace.h>

DEFINE_LOG_CATEGORY(LogSpatialSpanIdStore);

using namespace SpatialGDK;
using namespace worker::c;

// ----- SpatialSpanIdCache -----

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

Trace_SpanId SpatialSpanIdCache::GetSpanId(const EntityComponentId& Id, const uint32 FieldId) const
{
	Trace_SpanId ReturnSpanId = Trace_SpanId();

	const FieldIdMap* SpanIdMap = EntityComponentFieldSpanIds.Find(Id);
	if (SpanIdMap == nullptr)
	{
		return ReturnSpanId;
	}

	const EntityComponentFieldIdSpanIdUpdate* UpdateSpanId = SpanIdMap->Find(FieldId);
	if (UpdateSpanId == nullptr)
	{
		return ReturnSpanId;
	}

	return UpdateSpanId->SpanId;
}

Trace_SpanId SpatialSpanIdCache::GetMostRecentSpanId(const EntityComponentId& Id) const
{
	const FieldIdMap* SpanIdMap = EntityComponentFieldSpanIds.Find(Id);
	if (SpanIdMap == nullptr)
	{
		return Trace_SpanId();
	}

	EntityComponentFieldIdSpanIdUpdate MostRecent;
	for (const auto& Pair : *SpanIdMap)
	{
		const EntityComponentFieldIdSpanIdUpdate& Data = Pair.Value;
		if (Data.UpdateTime > MostRecent.UpdateTime || Trace_SpanId_Equal(MostRecent.SpanId, Trace_SpanId()))
		{
			MostRecent = Data;
		}
	}

	return MostRecent.SpanId;
}

void SpatialSpanIdCache::ClearSpanIds()
{
	EntityComponentFieldSpanIds.Empty();
}

// ----- SpatialTimedSpanIdCache -----

SpatialTimedSpanIdCache::SpatialTimedSpanIdCache()
{
	UpdateNextClearTime();
}

SpatialTimedSpanIdCache::SpatialTimedSpanIdCache(float InDropFrequency, float InMinSpanIdLifetime, int32 InMaxSpanIdsToDrop)
	: DropFrequency(InDropFrequency)
	, MinSpanIdLifetime(InMinSpanIdLifetime)
	, MaxSpanIdsToDrop(InMaxSpanIdsToDrop)
{
	UpdateNextClearTime();
}

void SpatialTimedSpanIdCache::DropOldSpanIds()
{
	if (NextClearTime < FDateTime::Now())
	{
		UpdateNextClearTime();

		FDateTime DropDateTime = FDateTime::Now() - FTimespan::FromSeconds(MinSpanIdLifetime);

		int32 NumDropped = 0;
		bool bShouldBreak = false;

		for (auto It = EntityComponentFieldSpanIds.CreateIterator(); It; ++It)
		{
			const EntityComponentId& Id = It.Key();
			FieldIdMap& Map = It.Value();

			for (auto FieldIdSpanIdPair = Map.CreateIterator(); FieldIdSpanIdPair; ++FieldIdSpanIdPair)
			{
				uint32 FieldId = FieldIdSpanIdPair.Key();
				const EntityComponentFieldIdSpanIdUpdate& SpanIdUpdate = FieldIdSpanIdPair.Value();

				if (SpanIdUpdate.UpdateTime < DropDateTime)
				{
					FieldIdSpanIdPair.RemoveCurrent();
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

		UE_LOG(LogSpatialSpanIdStore, Verbose, TEXT("Periodic SpanId drop dropped %d SpanIds."), NumDropped);

		EntityComponentFieldSpanIds.Compact();
	}
}

void SpatialTimedSpanIdCache::UpdateNextClearTime()
{
	NextClearTime = FDateTime::Now() + FTimespan::FromSeconds(DropFrequency);
}

// ----- SpatialWorkerOpSpanIdCache -----

void SpatialWorkerOpSpanIdCache::ComponentAdd(const Worker_Op& Op)
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

bool SpatialWorkerOpSpanIdCache::ComponentRemove(const Worker_Op& Op)
{
	const Worker_RemoveComponentOp& RemoveComponentOp = Op.op.remove_component;
	EntityComponentId Id(RemoveComponentOp.entity_id, RemoveComponentOp.component_id);
	return DropSpanIds(Id);
}

TArray<SpatialWorkerOpSpanIdCache::FieldSpanIdUpdate> SpatialWorkerOpSpanIdCache::ComponentUpdate(const Worker_Op& Op)
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
		Trace_SpanId ExistingSpanId = GetSpanId(Id, FieldId);
		if (!Trace_SpanId_Equal(ExistingSpanId, worker::c::Trace_SpanId()))
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
