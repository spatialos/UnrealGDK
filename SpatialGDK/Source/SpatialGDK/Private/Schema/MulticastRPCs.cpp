// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/MulticastRPCs.h"

namespace SpatialGDK
{
MulticastRPCs::MulticastRPCs(const Worker_ComponentData& Data)
	: MulticastRPCBuffer(ERPCType::NetMulticast)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data.schema_type));
}

void MulticastRPCs::ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update.schema_type));
}

void MulticastRPCs::ReadFromSchema(Schema_Object* SchemaObject)
{
	RPCRingBufferUtils::ReadBufferFromSchema(SchemaObject, MulticastRPCBuffer);

	// This is a special field that is set when creating a MulticastRPCs component with initial RPCs.
	// The server that first gains authority over the component will set last sent RPC ID to be equal
	// to this so the clients that already checked out this entity can execute initial RPCs.
	Schema_FieldId FieldId = RPCRingBufferUtils::GetInitiallyPresentMulticastRPCsCountFieldId();
	if (Schema_GetUint32Count(SchemaObject, FieldId) > 0)
	{
		InitiallyPresentMulticastRPCsCount = Schema_GetUint32(SchemaObject, FieldId);
	}
}

} // namespace SpatialGDK
