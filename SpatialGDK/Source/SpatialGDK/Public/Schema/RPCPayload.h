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

	uint32 Offset;
	uint32 Index;
	TArray<uint8> PayloadData;
};

struct RPCsOnEntityCreation : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::RPCS_ON_ENTITY_CREATION_ID;

	RPCsOnEntityCreation() = default;

	bool HasRPCPayloadData() const
	{
		return RPCs.Num() > 0;
	}

	RPCsOnEntityCreation(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentsObject = Schema_GetComponentDataFields(Data.schema_type);

		uint32 RPCCount = Schema_GetObjectCount(ComponentsObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID);

		for (uint32 i = 0; i < RPCCount; i++)
		{
			Schema_Object* ComponentObject = Schema_IndexObject(ComponentsObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID, i);
			RPCs.Add(RPCPayload(ComponentObject));
		}
	}

	Worker_ComponentData CreateRPCPayloadData() const
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		for (const auto& Payload : RPCs)
		{
			Schema_Object* Obj = Schema_AddObject(ComponentObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID);
			RPCPayload::WriteToSchemaObject(Obj, Payload.Offset, Payload.Index, Payload.PayloadData.GetData(), Payload.PayloadData.Num());
		}

		return Data;
	}

	static Worker_ComponentUpdate CreateClearFieldsUpdate()
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate(ComponentId);
		Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);
		Schema_AddComponentUpdateClearedField(Update.schema_type, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID);

		return Update;
	}

	static Worker_CommandRequest CreateClearFieldsCommandRequest()
	{
		Worker_CommandRequest CommandRequest = {};
		CommandRequest.component_id = ComponentId;
		CommandRequest.schema_type = Schema_CreateCommandRequest(ComponentId, SpatialConstants::CLEAR_RPCS_ON_ENTITY_CREATION);
		return CommandRequest;
	}

	TArray<RPCPayload> RPCs;
};

} // namespace SpatialGDK
