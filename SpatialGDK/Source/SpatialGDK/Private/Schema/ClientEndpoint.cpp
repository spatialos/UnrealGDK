// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ClientEndpoint.h"

namespace SpatialGDK
{
ClientEndpoint::ClientEndpoint(Schema_ComponentData* Data)
	: ReliableRPCBuffer(ERPCType::ServerReliable)
	, UnreliableRPCBuffer(ERPCType::ServerUnreliable)
	, AlwaysWriteRPCBuffer(ERPCType::ServerAlwaysWrite)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data));
}

void ClientEndpoint::ApplyComponentUpdate(Schema_ComponentUpdate* Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update));
}

void ClientEndpoint::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ReliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, UnreliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, AlwaysWriteRPCBuffer);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ClientReliable, ReliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ClientUnreliable, UnreliableRPCAck);
}

} // namespace SpatialGDK
