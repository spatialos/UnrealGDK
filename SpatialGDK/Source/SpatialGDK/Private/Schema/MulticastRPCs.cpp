// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/MulticastRPCs.h"

namespace SpatialGDK
{

MulticastRPCs::MulticastRPCs(const Worker_ComponentData& Data)
{
	InitBuffers();
	ReadFromSchema(Schema_GetComponentDataFields(Data.schema_type));
}

void MulticastRPCs::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update.schema_type));
}

void MulticastRPCs::InitBuffers()
{
	MulticastRPCBuffer.RingBuffer.SetNum(RPCRingBufferUtils::GetRingBufferSize(ERPCType::NetMulticast));
}

void MulticastRPCs::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ERPCType::NetMulticast, MulticastRPCBuffer);
}

} // namespace SpatialGDK
