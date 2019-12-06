// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ServerEndpoint.h"

namespace SpatialGDK
{

ServerEndpoint::ServerEndpoint(const Worker_ComponentData& Data)
{
	ReliableRPCBuffer.RingBuffer.SetNum(RPCRingBufferUtils::GetRingBufferSize(ERPCType::ClientReliable));
	UnreliableRPCBuffer.RingBuffer.SetNum(RPCRingBufferUtils::GetRingBufferSize(ERPCType::ClientUnreliable));
	ReadFromSchema(Schema_GetComponentDataFields(Data.schema_type));
}

void ServerEndpoint::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update.schema_type));
}

void ServerEndpoint::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ERPCType::ClientReliable, ReliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ERPCType::ClientUnreliable, UnreliableRPCBuffer);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ServerReliable, ReliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ServerUnreliable, UnreliableRPCAck);
}

} // namespace SpatialGDK
