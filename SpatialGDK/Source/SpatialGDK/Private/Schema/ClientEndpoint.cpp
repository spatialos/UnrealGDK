// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/ClientEndpoint.h"

#include "SpatialGDKSettings.h"

namespace SpatialGDK
{

ClientEndpoint::ClientEndpoint(const Worker_ComponentData& Data)
{
	InitBuffers();
	ReadFromSchema(Schema_GetComponentDataFields(Data.schema_type));
}

void ClientEndpoint::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update.schema_type));
}

void ClientEndpoint::InitBuffers()
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	ReliableRPCBuffer.RingBuffer.SetNum(SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ServerReliable));
	UnreliableRPCBuffer.RingBuffer.SetNum(SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ServerUnreliable));
}

void ClientEndpoint::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ServerReliable), ReliableRPCBuffer);
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ServerUnreliable), UnreliableRPCBuffer);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ClientReliable), ReliableRPCAck);
	RPCRingBufferUtils::ReadAckFromSchema(SchemaObject, RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::ClientUnreliable), UnreliableRPCAck);
}

} // namespace SpatialGDK
