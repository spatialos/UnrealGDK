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
struct RPCPayload
{
	RPCPayload() = delete;

	RPCPayload(uint32 InOffset, uint32 InIndex, uint32 InUniqueId, TArray<uint8>&& Data, TraceKey InTraceKey = InvalidTraceKey)
		: Offset(InOffset)
		, Index(InIndex)
		, UniqueId(InUniqueId)
		, PayloadData(MoveTemp(Data))
		, Trace(InTraceKey)
	{
	}

	RPCPayload(Schema_Object* RPCObject)
	{
		Offset = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID);
		Index = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID);
		UniqueId = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_UNIQUE_ID);
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
		WriteToSchemaObject(RPCObject, Offset, Index, UniqueId, PayloadData.GetData(), PayloadData.Num());

#if TRACE_LIB_ACTIVE
		if (USpatialLatencyTracer* Tracer = USpatialLatencyTracer::GetTracer(nullptr))
		{
			Tracer->WriteTraceToSchemaObject(Trace, RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_TRACE_ID);
		}
#endif
	}

	static void WriteToSchemaObject(Schema_Object* RPCObject, uint32 Offset, uint32 Index, const uint32 UniqueId, const uint8* Data,
									int32 NumElems)
	{
		Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID, Offset);
		Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID, Index);
		Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_UNIQUE_ID, UniqueId);
		AddBytesToSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID, Data, sizeof(uint8) * NumElems);
	}

	uint32 Offset;
	uint32 Index;
	uint32 UniqueId;
	TArray<uint8> PayloadData;
	TraceKey Trace = InvalidTraceKey;
};

} // namespace SpatialGDK
