// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ServerEndpoint.h"

namespace SpatialGDK
{
ServerEndpoint::ServerEndpoint(Schema_ComponentData* Data)
	: ClientReliableRPCBuffer(ERPCType::ClientReliable)
	, ClientUnreliableRPCBuffer(ERPCType::ClientUnreliable)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data));
}

void ServerEndpoint::ApplyComponentUpdate(Schema_ComponentUpdate* Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update));
}

void ServerEndpoint::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ClientReliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, ClientUnreliableRPCBuffer);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ServerReliable, ServerReliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::ServerUnreliable, ServerUnreliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, ERPCType::Movement, MovementRPCAck);
}

ServerEndpointLayout::ServerEndpointLayout()
	: ClientReliableRPCBufferDescriptor(1, RPCRingBufferUtils::GetRingBufferSize(ERPCType::ClientReliable))
	, ClientUnreliableRPCBufferDescriptor(ClientReliableRPCBufferDescriptor.GetSchemaFieldEnd() + 1,
										  RPCRingBufferUtils::GetRingBufferSize(ERPCType::ClientUnreliable))
	, ServerReliableRPCAckFieldId(ClientUnreliableRPCBufferDescriptor.GetSchemaFieldEnd() + 1)
	, ServerUnreliableRPCAckFieldId(ServerReliableRPCAckFieldId + 1)
	, MovementRPCAckFieldId(ServerUnreliableRPCAckFieldId + 1)
{
	Schema_FieldId Current;
	{
		// set up ClientReliableRPCBufferDescriptor
		// Current = ClientReliableRPCBufferDescriptor.GetSchemaFieldEnd() + 1;
	}
}

RPCRingBufferDescriptor ServerEndpointLayout::GetRingBufferDescriptor(ERPCType Type) const
{
	switch (Type)
	{
	case ERPCType::ClientReliable:
		return ClientReliableRPCBufferDescriptor;
	case ERPCType::ClientUnreliable:
		return ClientUnreliableRPCBufferDescriptor;
	default:
		checkNoEntry();
		return RPCRingBufferDescriptor();
	}
}

Schema_FieldId ServerEndpointLayout::GetAckFieldId(ERPCType Type) const
{
	switch (Type)
	{
	case ERPCType::ServerReliable:
		return ServerReliableRPCAckFieldId;
	case ERPCType::ServerUnreliable:
		return ServerUnreliableRPCAckFieldId;
	case ERPCType::Movement:
		return MovementRPCAckFieldId;
	default:
		checkNoEntry();
		return 0;
	}
}

} // namespace SpatialGDK
