// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ServerEndpoint.h"

#include "SpatialGDKSettings.h"

namespace SpatialGDK
{

ServerEndpoint::ServerEndpoint(const Worker_ComponentData& Data)
{
	InitBuffers();
	ReadFromSchema(Schema_GetComponentDataFields(Data.schema_type));
}

void ServerEndpoint::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update.schema_type));
}

void ServerEndpoint::InitBuffers()
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	ReliableRPCBuffer.RingBuffer.SetNum(SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ClientReliable));
	UnreliableRPCBuffer.RingBuffer.SetNum(SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ClientUnreliable));
}

void ServerEndpoint::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ClientReliable), ReliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ClientUnreliable), UnreliableRPCBuffer);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ServerReliable), ReliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ServerUnreliable), UnreliableRPCAck);
}

} // namespace SpatialGDK
