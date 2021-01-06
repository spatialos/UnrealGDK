// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/CrossServerEndpoint.h"

namespace SpatialGDK
{
CrossServerEndpoint::CrossServerEndpoint(Schema_ComponentData* Data)
	: ReliableRPCBuffer(ERPCType::CrossServer)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data));
}

void CrossServerEndpoint::ApplyComponentUpdate(Schema_ComponentUpdate* Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update));
	uint32 ClearCount = Schema_GetComponentUpdateClearedFieldCount(Update);
	TArray<Schema_FieldId> ClearedFields;
	ClearedFields.SetNum(ClearCount);
	Schema_GetComponentUpdateClearedFieldList(Update, ClearedFields.GetData());

	for (auto Field : ClearedFields)
	{
		uint32 SlotIdx = (Field - 1);
		if (SlotIdx % 2 == 0)
		{
			ReliableRPCBuffer.RingBuffer[SlotIdx / 2].Reset();
			ReliableRPCBuffer.Counterpart[SlotIdx / 2].Reset();
		}
	}
}

void CrossServerEndpoint::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ReliableRPCBuffer);
}

CrossServerEndpointACK::CrossServerEndpointACK(Schema_ComponentData* Data)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data));
}

void CrossServerEndpointACK::ApplyComponentUpdate(Schema_ComponentUpdate* Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update));
	uint32 ClearCount = Schema_GetComponentUpdateClearedFieldCount(Update);
	TArray<Schema_FieldId> ClearedFields;
	ClearedFields.SetNum(ClearCount);
	Schema_GetComponentUpdateClearedFieldList(Update, ClearedFields.GetData());

	for (auto Field : ClearedFields)
	{
		if (Field >= 1)
		{
			uint32 SlotIdx = (Field - 1);
			ACKArray[SlotIdx].Reset();
		}
	}
}

void ACKItem::ReadFromSchema(Schema_Object* SchemaObject)
{
	Sender = Schema_GetEntityId(SchemaObject, 1);
	RPCId = Schema_GetUint64(SchemaObject, 2);
	Result = Schema_GetUint64(SchemaObject, 3);
}

void ACKItem::WriteToSchema(Schema_Object* SchemaObject)
{
	Schema_AddEntityId(SchemaObject, 1, Sender);
	Schema_AddUint64(SchemaObject, 2, RPCId);
	Schema_AddUint64(SchemaObject, 3, Result);
}

void CrossServerEndpointACK::ReadFromSchema(Schema_Object* SchemaObject)
{
	uint32 Count = RPCRingBufferUtils::GetRingBufferSize(ERPCType::CrossServer);
	ACKArray.SetNum(Count);
	for (uint32 ACKIdx = 0; ACKIdx < Count; ++ACKIdx)
	{
		uint32 OptCount = Schema_GetObjectCount(SchemaObject, 1 + ACKIdx);
		if (OptCount > 0)
		{
			Schema_Object* ACKItemObject = Schema_GetObject(SchemaObject, 1 + ACKIdx);
			ACKArray[ACKIdx].Emplace(ACKItem());
			ACKArray[ACKIdx].GetValue().ReadFromSchema(ACKItemObject);
		}
	}
}

} // namespace SpatialGDK
