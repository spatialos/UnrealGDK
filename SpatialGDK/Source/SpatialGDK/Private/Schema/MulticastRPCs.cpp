// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/MulticastRPCs.h"

namespace SpatialGDK
{
MulticastRPCs::MulticastRPCs(Schema_ComponentData* Data)
	: MulticastRPCBuffer(ERPCType::NetMulticast)
{
	ReadFromSchema(Schema_GetComponentDataFields(Data));
}

void MulticastRPCs::ApplyComponentUpdate(Schema_ComponentUpdate* Update)
{
	ReadFromSchema(Schema_GetComponentUpdateFields(Update));
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

MulticastRPCsLayout::MulticastRPCsLayout()
	: MulticastRPCBufferDescriptor(1, RPCRingBufferUtils::GetRingBufferSize(ERPCType::NetMulticast))
	, InitiallyPresentMulticastRPCsCountFieldId(MulticastRPCBufferDescriptor.GetSchemaFieldEnd() + 1)
{
}

RPCRingBufferDescriptor MulticastRPCsLayout::GetRingBufferDescriptor(ERPCType Type) const
{
	switch (Type)
	{
	case ERPCType::Multicast:
		return MulticastRPCBufferDescriptor;
	default:
		checkNoEntry();
		return RPCRingBufferDescriptor();
	}
}

void MulticastRPCsLayout::MoveLastSentIdToInitiallyPresentCount(Schema_Object* SchemaObject, uint64 LastSentId)
{
	// This is a special field that is set when creating a MulticastRPCs component with initial RPCs.
	// Last sent RPC Id is cleared so the clients don't ignore the initial RPCs.
	// The server that first gains authority over the component will set last sent RPC ID to be equal
	// to the initial count so the clients that already checked out this entity can execute initial RPCs.
	Schema_ClearField(SchemaObject, MulticastRPCBufferDescriptor.LastSentRPCFieldId);
	Schema_AddUint32(SchemaObject, InitiallyPresentMulticastRPCsCountFieldId, static_cast<uint32>(LastSentId));
}

} // namespace SpatialGDK
