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
	NextClearTime = FDateTime::Now();
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
		AddEntityComponentFieldSpanId(Id, FieldId, Op.span_id);
	}
}

bool SpatialSpanIdStore::ComponentRemove(const Worker_Op& Op)
{
	const Worker_RemoveComponentOp& RemoveComponentOp = Op.op.remove_component;
	EntityComponentId Id(RemoveComponentOp.entity_id, RemoveComponentOp.component_id);
	return 	ClearEntityComponentSpanIds(Id);
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
		AddEntityComponentFieldSpanId(Id, FieldId, Op.span_id);
	}
}

worker::c::Trace_SpanId SpatialSpanIdStore::GetEntityComponentFieldSpanId(const EntityComponentId& Id, const uint32 FieldId)
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
	RemoveEntityComponentFieldInternal(SpanIdMap, Id, FieldId);

	return ReturnSpanId;
}

bool SpatialSpanIdStore::ClearEntityComponentSpanIds(const EntityComponentId& Id)
{
	return EntityComponentFieldSpanIds.Remove(Id);
}

bool SpatialSpanIdStore::RemoveEntityComponentFieldSpanId(const EntityComponentId& Id, const uint32 FieldId)
{
	return RemoveEntityComponentFieldInternal(EntityComponentFieldSpanIds.Find(Id), Id, FieldId);
}

bool SpatialSpanIdStore::RemoveEntityComponentFieldInternal(FieldIdMap* SpanIdMap, const EntityComponentId& Id, const uint32 FieldId)
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

void SpatialSpanIdStore::DropOldUpdates()
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
			RemoveEntityComponentFieldSpanId(Id.EntityComponentId, Id.FieldId);
		}

		EntityComponentFieldSpanIds.Compact();
	}
}

void SpatialSpanIdStore::ProcessRPCComponentUpdate(const EntityComponentId& Id, Schema_Object* SchemaObject, ERPCType RPCType, worker::c::Trace_SpanId SpanId)
{
	RPCRingBufferDescriptor Descriptor = RPCRingBufferUtils::GetRingBufferDescriptor(RPCType);
	for (uint32 RingBufferIndex = 0; RingBufferIndex < Descriptor.RingBufferSize; RingBufferIndex++)
	{
		Schema_FieldId FieldId = Descriptor.SchemaFieldStart + RingBufferIndex;
		if (Schema_GetObjectCount(SchemaObject, FieldId) > 0)
		{
			RPCPayload Payload = Schema_GetObject(SchemaObject, FieldId);
			AddEntityComponentFieldSpanId(Id, Payload.Index, SpanId);
		}
	}
}

void SpatialSpanIdStore::AddEntityComponentFieldSpanId(const EntityComponentId& Id, const uint32 FieldId, worker::c::Trace_SpanId SpanId)
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

bool SpatialSpanIdStore::IsComponentIdRPCEndpoint(const Worker_ComponentId ComponentId)
{
	return ComponentId == SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID || ComponentId == SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID;
}

void SpatialSpanIdStore::UpdateNextClearTime()
{
	NextClearTime = FDateTime::Now() + FTimespan::FromSeconds(ClearFrequency);
}
