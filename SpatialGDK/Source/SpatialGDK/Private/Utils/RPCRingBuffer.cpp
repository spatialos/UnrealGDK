// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/RPCRingBuffer.h"

#include "SpatialGDKSettings.h"

namespace SpatialGDK
{

namespace RPCRingBufferUtils
{

Worker_ComponentId GetRingBufferComponentId(ERPCType Type)
{
	switch (Type)
	{
	case ERPCType::ClientReliable:
	case ERPCType::ClientUnreliable:
		return SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID;
	case ERPCType::ServerReliable:
	case ERPCType::ServerUnreliable:
		return SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID;
	case ERPCType::NetMulticast:
		return SpatialConstants::MULTICAST_RPCS_COMPONENT_ID;
	default:
		checkNoEntry();
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}

RPCRingBufferDescriptor GetRingBufferDescriptor(ERPCType Type)
{
	RPCRingBufferDescriptor Descriptor;
	Descriptor.RingBufferSize = GetRingBufferSize(Type);

	uint32 MaxRingBufferSize = GetDefault<USpatialGDKSettings>()->MaxRPCRingBufferSize;
	switch (Type)
	{
	case ERPCType::ClientReliable:
	case ERPCType::ServerReliable:
	case ERPCType::NetMulticast:
		Descriptor.SchemaFieldStart = 1;
		Descriptor.LastSentRPCFieldId = 1 + MaxRingBufferSize;
		break;
	case ERPCType::ClientUnreliable:
	case ERPCType::ServerUnreliable:
		Descriptor.SchemaFieldStart = 1 + MaxRingBufferSize + 1;
		Descriptor.LastSentRPCFieldId = 1 + MaxRingBufferSize + 1 + MaxRingBufferSize;
		break;
	default:
		checkNoEntry();
		break;
	}

	return Descriptor;
}

uint32 GetRingBufferSize(ERPCType Type)
{
	return GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(Type);
}

Worker_ComponentId GetAckComponentId(ERPCType Type)
{
	switch (Type)
	{
	case ERPCType::ClientReliable:
	case ERPCType::ClientUnreliable:
		return SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID;
	case ERPCType::ServerReliable:
	case ERPCType::ServerUnreliable:
		return SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID;
	default:
		checkNoEntry();
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}

Schema_FieldId GetAckFieldId(ERPCType Type)
{
	uint32 MaxRingBufferSize = GetDefault<USpatialGDKSettings>()->MaxRPCRingBufferSize;

	switch (Type)
	{
	case ERPCType::ClientReliable:
	case ERPCType::ServerReliable:
		// In the generated schema components, acks will follow two ring buffers, each containing MaxRingBufferSize elements as well as a last sent ID.
		return 1 + 2 * (MaxRingBufferSize + 1);
	case ERPCType::ClientUnreliable:
	case ERPCType::ServerUnreliable:
		return 1 + 2 * (MaxRingBufferSize + 1) + 1;
	default:
		checkNoEntry();
		return 0;
	}
}

bool ShouldQueueOverflowed(ERPCType Type)
{
	switch (Type)
	{
	case ERPCType::ClientReliable:
	case ERPCType::ServerReliable:
		return true;
	case ERPCType::ClientUnreliable:
	case ERPCType::ServerUnreliable:
	case ERPCType::NetMulticast:
		return false;
	default:
		checkNoEntry();
		return false;
	}
}

void ReadBufferFromSchema(Schema_Object* SchemaObject, ERPCType Type, RPCRingBuffer& OutBuffer)
{
	RPCRingBufferDescriptor Descriptor = GetRingBufferDescriptor(Type);

	for (uint32 RingBufferIndex = 0; RingBufferIndex < Descriptor.RingBufferSize; RingBufferIndex++)
	{
		Schema_FieldId FieldId = Descriptor.SchemaFieldStart + RingBufferIndex;
		if (Schema_GetObjectCount(SchemaObject, FieldId) > 0)
		{
			OutBuffer.RingBuffer[RingBufferIndex].Emplace(Schema_GetObject(SchemaObject, FieldId));
		}
	}

	if (Schema_GetUint64Count(SchemaObject, Descriptor.LastSentRPCFieldId) > 0)
	{
		OutBuffer.LastSentRPCId = Schema_GetUint64(SchemaObject, Descriptor.LastSentRPCFieldId);
	}
}

void ReadAckFromSchema(Schema_Object* SchemaObject, ERPCType Type, uint64& OutAck)
{
	Schema_FieldId AckFieldId = GetAckFieldId(Type);

	if (Schema_GetUint64Count(SchemaObject, AckFieldId) > 0)
	{
		OutAck = Schema_GetUint64(SchemaObject, AckFieldId);
	}
}

void WriteRPCToSchema(Schema_Object* SchemaObject, ERPCType Type, uint64 RPCId, RPCPayload Payload)
{
	RPCRingBufferDescriptor Descriptor = GetRingBufferDescriptor(Type);

	Schema_Object* RPCObject = Schema_AddObject(SchemaObject, Descriptor.GetRingBufferElementFieldId(RPCId));
	RPCPayload::WriteToSchemaObject(RPCObject, Payload.Offset, Payload.Index, Payload.PayloadData.GetData(), Payload.PayloadData.Num());

	Schema_ClearField(SchemaObject, Descriptor.LastSentRPCFieldId);
	Schema_AddUint64(SchemaObject, Descriptor.LastSentRPCFieldId, RPCId);
}

void WriteAckToSchema(Schema_Object* SchemaObject, ERPCType Type, uint64 Ack)
{
	Schema_FieldId AckFieldId = GetAckFieldId(Type);

	Schema_ClearField(SchemaObject, AckFieldId);
	Schema_AddUint64(SchemaObject, AckFieldId, Ack);
}

} // namespace RPCRingBufferUtils

} // namespace SpatialGDK
