// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/RPCRingBuffer.h"

#include "SpatialGDKSettings.h"

namespace SpatialGDK
{
RPCRingBuffer::RPCRingBuffer(ERPCType InType)
	: Type(InType)
{
	RingBuffer.SetNum(RPCRingBufferUtils::GetRingBufferSize(Type));
}

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
	// In schema, the client and server endpoints will first have a
	//   Reliable ring buffer, starting from 1 and containing MaxRingBufferSize elements, then
	//   Last sent reliable RPC,
	//   Unreliable ring buffer, containing MaxRingBufferSize elements,
	//   Last sent unreliable RPC,
	//   followed by reliable and unreliable RPC acks.
	// MulticastRPCs component will only have one buffer that looks like the reliable buffer above.
	// The numbers below are based on this structure, and have to match the component generated in SchemaGenerator
	// (GenerateRPCEndpointsSchema).
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
		// In the generated schema components, acks will follow two ring buffers, each containing MaxRingBufferSize elements as well as a
		// last sent ID.
		return 1 + 2 * (MaxRingBufferSize + 1);
	case ERPCType::ClientUnreliable:
	case ERPCType::ServerUnreliable:
		return 1 + 2 * (MaxRingBufferSize + 1) + 1;
	default:
		checkNoEntry();
		return 0;
	}
}

Schema_FieldId GetInitiallyPresentMulticastRPCsCountFieldId()
{
	uint32 MaxRingBufferSize = GetDefault<USpatialGDKSettings>()->MaxRPCRingBufferSize;
	// This field directly follows the ring buffer + last sent id.
	return 1 + MaxRingBufferSize + 1;
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

void ReadBufferFromSchema(Schema_Object* SchemaObject, RPCRingBuffer& OutBuffer)
{
	RPCRingBufferDescriptor Descriptor = GetRingBufferDescriptor(OutBuffer.Type);

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

void ReadAckFromSchema(const Schema_Object* SchemaObject, ERPCType Type, uint64& OutAck)
{
	Schema_FieldId AckFieldId = GetAckFieldId(Type);

	if (Schema_GetUint64Count(SchemaObject, AckFieldId) > 0)
	{
		OutAck = Schema_GetUint64(SchemaObject, AckFieldId);
	}
}

void WriteRPCToSchema(Schema_Object* SchemaObject, ERPCType Type, uint64 RPCId, const RPCPayload& Payload)
{
	RPCRingBufferDescriptor Descriptor = GetRingBufferDescriptor(Type);

	Schema_Object* RPCObject = Schema_AddObject(SchemaObject, Descriptor.GetRingBufferElementFieldId(RPCId));
	Payload.WriteToSchemaObject(RPCObject);

	Schema_ClearField(SchemaObject, Descriptor.LastSentRPCFieldId);
	Schema_AddUint64(SchemaObject, Descriptor.LastSentRPCFieldId, RPCId);
}

void WriteAckToSchema(Schema_Object* SchemaObject, ERPCType Type, uint64 Ack)
{
	Schema_FieldId AckFieldId = GetAckFieldId(Type);

	Schema_ClearField(SchemaObject, AckFieldId);
	Schema_AddUint64(SchemaObject, AckFieldId, Ack);
}

void MoveLastSentIdToInitiallyPresentCount(Schema_Object* SchemaObject, uint64 LastSentId)
{
	// This is a special field that is set when creating a MulticastRPCs component with initial RPCs.
	// Last sent RPC Id is cleared so the clients don't ignore the initial RPCs.
	// The server that first gains authority over the component will set last sent RPC ID to be equal
	// to the initial count so the clients that already checked out this entity can execute initial RPCs.
	RPCRingBufferDescriptor Descriptor = GetRingBufferDescriptor(ERPCType::NetMulticast);
	Schema_ClearField(SchemaObject, Descriptor.LastSentRPCFieldId);
	Schema_AddUint32(SchemaObject, GetInitiallyPresentMulticastRPCsCountFieldId(), static_cast<uint32>(LastSentId));
}

} // namespace RPCRingBufferUtils

} // namespace SpatialGDK
