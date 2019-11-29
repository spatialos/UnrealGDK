// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/MulticastRPCs.h"

#include "SpatialGDKSettings.h"

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
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	MulticastRPCBuffer.RingBuffer.SetNum(SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::NetMulticast));
}

void MulticastRPCs::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, RPCRingBufferUtils::GetRingBufferDescriptor(ERPCType::NetMulticast), MulticastRPCBuffer);
}

} // namespace SpatialGDK
