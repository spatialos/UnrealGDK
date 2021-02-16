// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/RPCRingBuffer.h"

#include "SpatialGDKSettings.h"

namespace SpatialGDK
{
RPCRingBuffer::RPCRingBuffer(ERPCType InType)
	: Type(InType)
{
	RingBuffer.SetNum(RPCRingBufferUtils::GetRingBufferSize(Type));
	if (InType == ERPCType::CrossServer)
	{
		Counterpart.SetNum(RPCRingBufferUtils::GetRingBufferSize(Type));
	}
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
	case ERPCType::ServerAlwaysWrite:
		return SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID;
	case ERPCType::NetMulticast:
		return SpatialConstants::MULTICAST_RPCS_COMPONENT_ID;
	default:
		checkNoEntry();
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}

Worker_ComponentId GetRingBufferAuthComponentSetId(ERPCType Type)
{
	switch (Type)
	{
	case ERPCType::ClientReliable:
	case ERPCType::ClientUnreliable:
	case ERPCType::NetMulticast:
		return SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID;
	case ERPCType::ServerReliable:
	case ERPCType::ServerUnreliable:
	case ERPCType::ServerAlwaysWrite:
		return SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID;
	default:
		checkNoEntry();
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}

RPCRingBufferDescriptor GetRingBufferDescriptor(ERPCType Type)
{
	RPCRingBufferDescriptor Descriptor = {};
	Descriptor.RingBufferSize = GetRingBufferSize(Type);

	const Schema_FieldId SchemaStart = 1;

	switch (Type)
	{
	case ERPCType::ClientReliable:
	case ERPCType::ServerReliable:
	case ERPCType::NetMulticast:
		// These buffers start at the beginning on their corresponding components.
		Descriptor.SchemaFieldStart = SchemaStart;
		Descriptor.LastSentRPCFieldId = Descriptor.SchemaFieldStart + Descriptor.RingBufferSize;
		break;

	case ERPCType::ClientUnreliable:
	{
		// ClientUnreliable buffer starts after ClientReliable. Add 1 to account for the last sent ID field.
		const uint32 ClientReliableBufferSize = GetRingBufferSize(ERPCType::ClientReliable) + 1;

		Descriptor.SchemaFieldStart = SchemaStart + ClientReliableBufferSize;
		Descriptor.LastSentRPCFieldId = Descriptor.SchemaFieldStart + Descriptor.RingBufferSize;
		break;
	}
	case ERPCType::ServerUnreliable:
	{
		// ServerUnreliable buffer starts after ServerReliable. Add 1 to account for the last sent ID field.
		const uint32 ServerReliableBufferSize = GetRingBufferSize(ERPCType::ServerReliable) + 1;

		Descriptor.SchemaFieldStart = SchemaStart + ServerReliableBufferSize;
		Descriptor.LastSentRPCFieldId = Descriptor.SchemaFieldStart + Descriptor.RingBufferSize;
		break;
	}
	case ERPCType::ServerAlwaysWrite:
	{
		// ServerAlwaysWrite buffer starts after ServerReliable and ServerUnreliable buffers.
		// Add 1 to each to account for the last sent ID fields.
		const uint32 ServerReliableBufferSize = GetRingBufferSize(ERPCType::ServerReliable) + 1;
		const uint32 ServerUnreliableBufferSize = GetRingBufferSize(ERPCType::ServerUnreliable) + 1;

		Descriptor.SchemaFieldStart = SchemaStart + ServerReliableBufferSize + ServerUnreliableBufferSize;
		Descriptor.LastSentRPCFieldId = Descriptor.SchemaFieldStart + Descriptor.RingBufferSize;
		break;
	}
	case ERPCType::CrossServer:
	{
		const uint32 BufferSize = GetRingBufferSize(ERPCType::CrossServer);
		Descriptor.SchemaFieldStart = 1;
		Descriptor.LastSentRPCFieldId = 1 + 2 * BufferSize;
		break;
	}
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
	case ERPCType::ServerAlwaysWrite:
		return SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID;
	default:
		checkNoEntry();
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}

Worker_ComponentId GetAckAuthComponentSetId(ERPCType Type)
{
	switch (Type)
	{
	case ERPCType::ClientReliable:
	case ERPCType::ClientUnreliable:
		return SpatialConstants::CLIENT_AUTH_COMPONENT_SET_ID;
	case ERPCType::ServerReliable:
	case ERPCType::ServerUnreliable:
	case ERPCType::ServerAlwaysWrite:
		return SpatialConstants::SERVER_AUTH_COMPONENT_SET_ID;
	default:
		checkNoEntry();
		return SpatialConstants::INVALID_COMPONENT_ID;
	}
}

Schema_FieldId GetAckFieldId(ERPCType Type)
{
	const Schema_FieldId SchemaStart = 1;

	switch (Type)
	{
	case ERPCType::ClientReliable:
	{
		// Client acks follow ServerReliable, ServerUnreliable, and ServerAlwaysWrite buffers.
		// Add 1 to each to account for the last sent ID fields.
		const uint32 ServerReliableBufferSize = GetRingBufferSize(ERPCType::ServerReliable) + 1;
		const uint32 ServerUnreliableBufferSize = GetRingBufferSize(ERPCType::ServerUnreliable) + 1;
		const uint32 ServerAlwaysWriteBufferSize = GetRingBufferSize(ERPCType::ServerAlwaysWrite) + 1;

		return SchemaStart + ServerReliableBufferSize + ServerUnreliableBufferSize + ServerAlwaysWriteBufferSize;
	}
	case ERPCType::ClientUnreliable:
		// Client Unreliable ack directly follows Reliable ack.
		return GetAckFieldId(ERPCType::ClientReliable) + 1;

	case ERPCType::ServerReliable:
	{
		// Server acks follow Client Reliable and Unreliable buffers.
		// Add 1 to each to account for the last sent ID fields.
		const uint32 ClientReliableBufferSize = GetRingBufferSize(ERPCType::ClientReliable) + 1;
		const uint32 ClientUnreliableBufferSize = GetRingBufferSize(ERPCType::ClientUnreliable) + 1;

		return SchemaStart + ClientReliableBufferSize + ClientUnreliableBufferSize;
	}
	case ERPCType::ServerUnreliable:
		// Server Unreliable ack directly follows Reliable ack.
		return GetAckFieldId(ERPCType::ServerReliable) + 1;

	case ERPCType::ServerAlwaysWrite:
		// Server Always Write ack directly follows Unreliable ack.
		return GetAckFieldId(ERPCType::ServerUnreliable) + 1;

	default:
		checkNoEntry();
		return 0;
	}
}

Schema_FieldId GetInitiallyPresentMulticastRPCsCountFieldId()
{
	// This field directly follows the ring buffer + last sent id.
	const Schema_FieldId SchemaStart = 1;
	// Add 1 to account for the last sent ID field.
	const uint32 MulticastBufferSize = GetRingBufferSize(ERPCType::NetMulticast) + 1;

	return SchemaStart + MulticastBufferSize;
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
	case ERPCType::ServerAlwaysWrite:
	case ERPCType::NetMulticast:
		return false;
	default:
		checkNoEntry();
		return false;
	}
}

bool ShouldIgnoreCapacity(ERPCType Type)
{
	switch (Type)
	{
	case ERPCType::ServerAlwaysWrite:
		return true;
	case ERPCType::ClientReliable:
	case ERPCType::ClientUnreliable:
	case ERPCType::ServerReliable:
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
		Schema_FieldId FieldId = Descriptor.GetRingBufferElementFieldId(OutBuffer.Type, RingBufferIndex + 1);
		if (Schema_GetObjectCount(SchemaObject, FieldId) > 0)
		{
			OutBuffer.RingBuffer[RingBufferIndex].Emplace(Schema_GetObject(SchemaObject, FieldId));
		}
		if (OutBuffer.Type == ERPCType::CrossServer)
		{
			if (Schema_GetObjectCount(SchemaObject, FieldId + 1) > 0)
			{
				OutBuffer.Counterpart[RingBufferIndex].Emplace(CrossServerRPCInfo::ReadFromSchema(SchemaObject, FieldId + 1));
			}
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

	Schema_Object* RPCObject = Schema_AddObject(SchemaObject, Descriptor.GetRingBufferElementFieldId(Type, RPCId));
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
