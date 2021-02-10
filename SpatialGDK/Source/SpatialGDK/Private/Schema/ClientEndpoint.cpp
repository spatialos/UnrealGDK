// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ClientEndpoint.h"

namespace SpatialGDK
{
ClientEndpoint::ClientEndpoint(Schema_ComponentData* Data)
	: ServerReliableRPCBuffer(ERPCType::ServerReliable)
	, ServerUnreliableRPCBuffer(ERPCType::ServerUnreliable)
	, MovementRPCBuffer(ERPCType::Movement)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data));
}

void ClientEndpoint::ApplyComponentUpdate(Schema_ComponentUpdate* Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update));
}

void ClientEndpoint::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ServerReliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ServerUnreliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, MovementRPCBuffer);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ClientReliable, ClientReliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ClientUnreliable, ClientUnreliableRPCAck);
}

ClientEndpointLayout::ClientEndpointLayout()
	: ServerReliableRPCBufferDescriptor(1, RPCRingBufferUtils::GetRingBufferSize(ERPCType::ServerReliable))
	, ServerUnreliableRPCBufferDescriptor(ServerReliableRPCBufferDescriptor.GetSchemaFieldEnd() + 1,
										  RPCRingBufferUtils::GetRingBufferSize(ERPCType::ServerUnreliable))
	, MovementRPCBufferDescriptor(ServerUnreliableRPCBufferDescriptor.GetSchemaFieldEnd() + 1,
								  RPCRingBufferUtils::GetRingBufferSize(ERPCType::Movement))
	, ClientReliableRPCAckFieldId(MovementRPCBufferDescriptor.GetSchemaFieldEnd() + 1)
	, ClientUnreliableRPCAckFieldId(ClientReliableRPCAckFieldId + 1)
{
}

RPCRingBufferDescriptor ClientEndpointLayout::GetRingBufferDescriptor(ERPCType Type) const
{
	switch (Type)
	{
	case ERPCType::ServerReliable:
		return ServerReliableRPCBufferDescriptor;
	case ERPCType::ServerUnreliable:
		return ServerUnreliableRPCBufferDescriptor;
	case ERPCType::Movement:
		return MovementRPCBufferDescriptor;
	default:
		checkNoEntry();
		return RPCRingBufferDescriptor();
	}
}

Schema_FieldId ClientEndpointLayout::GetAckFieldId(ERPCType Type) const
{
	switch (Type)
	{
	case ERPCType::ClientReliable:
		return ClientReliableRPCAckFieldId;
	case ERPCType::ClientUnreliable:
		return ClientUnreliableRPCAckFieldId;
	default:
		checkNoEntry();
		return 0;
	}
}

} // namespace SpatialGDK
