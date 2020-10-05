// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/CrossServerEndpoint.h"

namespace SpatialGDK
{
CrossServerEndpoint::CrossServerEndpoint(const Worker_ComponentData& Data, ERPCType Type)
	: ReliableRPCBuffer(Type)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data.schema_type));
}

void CrossServerEndpoint::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update.schema_type));
	uint32 ClearCount = Schema_GetComponentUpdateClearedFieldCount(Update.schema_type);
	TArray<Schema_FieldId> ClearedFields;
	ClearedFields.SetNum(ClearCount);
	Schema_GetComponentUpdateClearedFieldList(Update.schema_type, ClearedFields.GetData());

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

CrossServerEndpointACK::CrossServerEndpointACK(const Worker_ComponentData& Data, ERPCType InType)
	: Type(InType)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data.schema_type));
}

void CrossServerEndpointACK::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update.schema_type));
	uint32 ClearCount = Schema_GetComponentUpdateClearedFieldCount(Update.schema_type);
	TArray<Schema_FieldId> ClearedFields;
	ClearedFields.SetNum(ClearCount);
	Schema_GetComponentUpdateClearedFieldList(Update.schema_type, ClearedFields.GetData());

	for (auto Field : ClearedFields)
	{
		if (Field >= 2)
		{
			uint32 SlotIdx = (Field - 2);
			ACKArray[SlotIdx].Reset();
		}
	}
}

void ACKItem::ReadFromSchema(Schema_Object* SchemaObject)
{
	Sender = Schema_GetEntityId(SchemaObject, 1);
	RPCId = Schema_GetUint64(SchemaObject, 2);
	SenderRevision = Schema_GetUint64(SchemaObject, 3);
}

void ACKItem::WriteToSchema(Schema_Object* SchemaObject)
{
	Schema_AddEntityId(SchemaObject, 1, Sender);
	Schema_AddUint64(SchemaObject, 2, RPCId);
	Schema_AddUint64(SchemaObject, 3, SenderRevision);
}

void CrossServerEndpointACK::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, Type, RPCAck);

	uint32 Count = RPCRingBufferUtils::GetRingBufferSize(Type);
	ACKArray.SetNum(Count);
	for (uint32 ACKIdx = 0; ACKIdx < Count; ++ACKIdx)
	{
		uint32 OptCount = Schema_GetObjectCount(SchemaObject, 2 + ACKIdx);
		if (OptCount > 0)
		{
			Schema_Object* ACKItemObject = Schema_GetObject(SchemaObject, 2 + ACKIdx);
			ACKArray[ACKIdx].Emplace(ACKItem());
			ACKArray[ACKIdx].GetValue().ReadFromSchema(ACKItemObject);
		}
	}
}

} // namespace SpatialGDK
