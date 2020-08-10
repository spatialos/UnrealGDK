// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ServerEndpoint.h"

namespace SpatialGDK
{
ServerEndpoint::ServerEndpoint(const Worker_ComponentData& Data)
	: ReliableRPCBuffer(ERPCType::ClientReliable)
	, UnreliableRPCBuffer(ERPCType::ClientUnreliable)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data.schema_type));
}

void ServerEndpoint::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update.schema_type));
}

void ServerEndpoint::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ReliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, UnreliableRPCBuffer);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ServerReliable, ReliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ServerUnreliable, UnreliableRPCAck);
}

} // namespace SpatialGDK
