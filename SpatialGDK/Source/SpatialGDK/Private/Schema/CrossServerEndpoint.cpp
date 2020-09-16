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
}

void CrossServerEndpointACK::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, Type, RPCAck);
}

CrossServerEndpointSenderACK::CrossServerEndpointSenderACK(const Worker_ComponentData& Data)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data.schema_type));
}

void CrossServerEndpointSenderACK::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update.schema_type));
}

void CrossServerEndpointSenderACK::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::CrossServerSender, RPCAck);

	uint32 Count = Schema_GetObjectCount(SchemaObject, 2);

	DottedRPCACK.Empty();
	for (uint32 EntryIdx = 0; EntryIdx < Count; ++EntryIdx)
	{
		Schema_Object* MapEntry = Schema_IndexObject(SchemaObject, 2, EntryIdx);

		Worker_EntityId Entity = Schema_GetEntityId(MapEntry, SCHEMA_MAP_KEY_FIELD_ID);

		Schema_Object* ACKListObject = Schema_GetObject(MapEntry, SCHEMA_MAP_VALUE_FIELD_ID);

		uint32 ACKCount = Schema_GetUint64Count(ACKListObject, 1);
		TArray<uint64> ACKList;
		ACKList.SetNum(ACKCount);
		Schema_GetUint64List(ACKListObject, 1, reinterpret_cast<uint64_t*>(ACKList.GetData()));

		DottedRPCACK.Add(Entity, MoveTemp(ACKList));
	}
}

void CrossServerEndpointSenderACK::CreateUpdate(Schema_ComponentUpdate* OutUpdate)
{
	Schema_Object* Object = Schema_GetComponentUpdateFields(OutUpdate);

	Schema_AddUint64(Object, 1, RPCAck);

	if (DottedRPCACK.Num() == 0)
	{
		Schema_AddComponentUpdateClearedField(OutUpdate, 2);
	}

	for (auto const& ACKEntry : DottedRPCACK)
	{
		Schema_Object* MapEntry = Schema_AddObject(Object, 2);

		Schema_AddEntityId(MapEntry, SCHEMA_MAP_KEY_FIELD_ID, ACKEntry.Key);
		Schema_Object* ACKListObject = Schema_AddObject(MapEntry, SCHEMA_MAP_VALUE_FIELD_ID);

		Schema_AddUint64List(ACKListObject, 1, reinterpret_cast<uint64_t const*>(ACKEntry.Value.GetData()), ACKEntry.Value.Num());
	}
}

} // namespace SpatialGDK
