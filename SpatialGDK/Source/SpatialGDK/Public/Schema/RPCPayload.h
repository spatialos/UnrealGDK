// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

#pragma optimize("", off)
namespace improbable
{

struct RPCPayload
{
	//RPCPayload() = default;
	RPCPayload() = delete;

	RPCPayload(uint32 InOffset, uint32 InIndex) : Offset(InOffset), Index(InIndex)
	{
	}

	RPCPayload(const Schema_Object* RPCObject)
	{
		Offset = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID);
		Index = Schema_GetUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID);
		PayloadData = improbable::GetBytesFromSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID);
	}

	void WriteToSchemaObject(Schema_Object* RPCObject, FBitWriter& Writer) const
	{
		Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_OFFSET_ID, Offset);
		Schema_AddUint32(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_INDEX_ID, Index);
		improbable::AddBytesToSchema(RPCObject, SpatialConstants::UNREAL_RPC_PAYLOAD_RPC_PAYLOAD_ID, Writer);
	}

	uint32 Offset;
	uint32 Index;
	// TODO: Copy later?
	TArray<uint8> PayloadData;
};

struct RPCsOnEntityCreation : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::RPC_ON_ENTITY_CREATION_ID;

	RPCsOnEntityCreation() = default;

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

	Worker_ComponentData CreateRPCPayloadData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		for (const auto& elem : RPCs)
		{
			FBitWriter DefaultWriter;
			Schema_Object* Obj = Schema_AddObject(ComponentObject, SpatialConstants::RPC_ON_ENTITY_CREATION_RPCS_ID);
			elem.WriteToSchemaObject(Obj, DefaultWriter);
		}

		return Data;
	}

	TArray<RPCPayload> RPCs;
};

}
#pragma optimize("", on)
