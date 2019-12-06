// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ClientEndpoint.h"

namespace SpatialGDK
{

ClientEndpoint::ClientEndpoint(const Worker_ComponentData& Data)
{
	ReliableRPCBuffer.RingBuffer.SetNum(RPCRingBufferUtils::GetRingBufferSize(ERPCType::ServerReliable));
	UnreliableRPCBuffer.RingBuffer.SetNum(RPCRingBufferUtils::GetRingBufferSize(ERPCType::ServerUnreliable));
	ReadFromSchema(Schema_GetComponentDataFields(Data.schema_type));
}

void ClientEndpoint::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update.schema_type));
}

void ClientEndpoint::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ERPCType::ServerReliable, ReliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ERPCType::ServerUnreliable, UnreliableRPCBuffer);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ClientReliable, ReliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ClientUnreliable, UnreliableRPCAck);
}

} // namespace SpatialGDK
