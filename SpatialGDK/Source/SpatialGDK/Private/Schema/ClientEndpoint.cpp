// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ClientEndpoint.h"

namespace SpatialGDK
{
ClientEndpoint::ClientEndpoint(const Worker_ComponentData& Data)
	: ReliableRPCBuffer(ERPCType::ServerReliable)
	, UnreliableRPCBuffer(ERPCType::ServerUnreliable)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data.schema_type));
}

void ClientEndpoint::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update.schema_type));
}

void ClientEndpoint::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ReliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, UnreliableRPCBuffer);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ClientReliable, ReliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ClientUnreliable, UnreliableRPCAck);
}

} // namespace SpatialGDK
