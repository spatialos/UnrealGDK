// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct RPCPayload
{
	RPCPayload() = delete;

	RPCPayload(uint32 InOffset, uint32 InIndex, TArray<uint8>&& Data) : Offset(InOffset), Index(InIndex), PayloadData(MoveTemp(Data))
	{}

	RPCPayload(const Schema_Object* RPCObject)
	{
		Offset = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID);
		Index = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID);
		PayloadData = SpatialGDK::GetBytesFromSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID);
	}

	int64 CountDataBits() const
	{
		return PayloadData.Num() * 8;
	}

	static void WriteToSchemaObject(Schema_Object* RPCObject, uint32 Offset, uint32 Index, const uint8* Data, int32 NumElems)
	{
		Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID, Offset);
		Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID, Index);
		AddBytesToSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID, Data, sizeof(uint8) * NumElems);
	}

	void WriteToSchemaObject(Schema_Object* RPCObject) const
	{
		WriteToSchemaObject(RPCObject, Offset, Index, PayloadData.GetData(), PayloadData.Num());
	}

	uint32 Offset;
	uint32 Index;
	TArray<uint8> PayloadData;
};

using QueuedRPCMap = TMap<ESchemaComponentType, TArray<RPCPayload>>;

} // namespace SpatialGDK
