// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"
#include "Utils/SpatialLatencyTracer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct CrossServerRPCInfo
{
	CrossServerRPCInfo()
		: Entity(SpatialConstants::INVALID_ENTITY_ID)
		, RPCId(0)
	{
	}
	CrossServerRPCInfo(Worker_EntityId InCounterpart, uint64 InRPCId)
		: Entity(InCounterpart)
		, RPCId(InRPCId)
	{
	}
	bool operator==(const CrossServerRPCInfo& iInfo) const { return Entity == iInfo.Entity && RPCId == iInfo.RPCId; }
	Worker_EntityId Entity;
	uint64 RPCId;

	static CrossServerRPCInfo ReadFromSchema(Schema_Object* Object, Schema_FieldId Id)
	{
		Schema_Object* InfoObject = Schema_GetObject(Object, Id);

		Worker_EntityId EntityId = Schema_GetEntityId(InfoObject, 1);
		uint64 RPCId = Schema_GetUint64(InfoObject, 2);

		return CrossServerRPCInfo(EntityId, RPCId);
	}

	void AddToSchema(Schema_Object* Object, Schema_FieldId Id) const
	{
		Schema_Object* InfoObject = Schema_AddObject(Object, Id);

		Schema_AddEntityId(InfoObject, 1, Entity);
		Schema_AddUint64(InfoObject, 2, RPCId);
	}
};

struct RPCTarget : CrossServerRPCInfo
{
	RPCTarget() = default;
	explicit RPCTarget(const CrossServerRPCInfo& iInfo)
		: CrossServerRPCInfo(iInfo)
	{
	}
};

struct RPCSender : CrossServerRPCInfo
{
	RPCSender() = default;
	RPCSender(Worker_EntityId Sender, uint64 RPCId)
		: CrossServerRPCInfo(Sender, RPCId)
	{
	}
	explicit RPCSender(const CrossServerRPCInfo& iInfo)
		: CrossServerRPCInfo(iInfo)
	{
	}
};

struct RPCPayload
{
	RPCPayload() = delete;

	RPCPayload(uint32 InOffset, uint32 InIndex, TOptional<uint64> InId, TArray<uint8>&& Data, TraceKey InTraceKey = InvalidTraceKey)
		: Offset(InOffset)
		, Index(InIndex)
		, Id(InId)
		, PayloadData(MoveTemp(Data))
		, Trace(InTraceKey)
	{
	}

	RPCPayload(Schema_Object* RPCObject)
	{
		Offset = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID);
		Index = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID);
		if (Schema_GetUint64Count(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_ID) > 0)
		{
			Id = Schema_GetUint64(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_ID);
		}

		PayloadData = GetBytesFromSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID);

#if TRACE_LIB_ACTIVE
		if (USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(nullptr))
		{
			Trace = Tracer->ReadTraceFromSchemaObject(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_TRACE_ID);
		}
#endif
	}

	int64 CountDataBits() const { return PayloadData.Num() * 8; }

	void WriteToSchemaObject(Schema_Object* RPCObject) const
	{
		WriteToSchemaObject(RPCObject, Offset, Index, Id, PayloadData.GetData(), PayloadData.Num());

#if TRACE_LIB_ACTIVE
		if (USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(nullptr))
		{
			Tracer->WriteTraceToSchemaObject(Trace, RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_TRACE_ID);
		}
#endif
	}

	static void WriteToSchemaObject(Schema_Object* RPCObject, uint32 Offset, uint32 Index, TOptional<uint64> UniqueId, const uint8* Data,
									int32 NumElems)
	{
		Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID, Offset);
		Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID, Index);
		if (UniqueId.IsSet())
		{
			Schema_AddUint64(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_ID, UniqueId.GetValue());
		}

		AddBytesToSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID, Data, sizeof(uint8) * NumElems);
	}

	uint32 Offset;
	uint32 Index;
	TOptional<uint64> Id;
	TArray<uint8> PayloadData;
	TraceKey Trace = InvalidTraceKey;
};

} // namespace SpatialGDK
