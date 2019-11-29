// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/RPCRingBuffer.h"

#include "SpatialGDKSettings.h"

namespace SpatialGDK
{

namespace RPCRingBufferUtils
{

RPCRingBufferDescriptor GetRingBufferDescriptor(ERPCType Type)
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();

	switch (Type)
	{
	case ERPCType::ClientReliable:
		return {
			ERPCType::ClientReliable,
			SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID, // buffer comp
			SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ClientReliable), // buffer size
			1, // field start
			1 + SpatialGDKSettings->MaxRPCRingBufferSize, // last sent field id
			SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, // ack comp
			SpatialGDKSettings->MaxRPCRingBufferSize * 2 + 2 + 1, // ack field id
			true // should queue overflowed
		};
	case ERPCType::ClientUnreliable:
		return {
			ERPCType::ClientUnreliable,
			SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID, // buffer comp
			SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ClientUnreliable), // buffer size
			1 + SpatialGDKSettings->MaxRPCRingBufferSize + 1, // field start
			1 + SpatialGDKSettings->MaxRPCRingBufferSize + 1 + SpatialGDKSettings->MaxRPCRingBufferSize, // last sent field id
			SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, // ack comp
			SpatialGDKSettings->MaxRPCRingBufferSize * 2 + 2 + 2, // ack field id
			false // should queue overflowed
		};
	case ERPCType::ServerReliable:
		return {
			ERPCType::ServerReliable,
			SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, // buffer comp
			SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ServerReliable), // buffer size
			1, // field start
			1 + SpatialGDKSettings->MaxRPCRingBufferSize, // last sent field id
			SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID, // ack comp
			SpatialGDKSettings->MaxRPCRingBufferSize * 2 + 2 + 1, // ack field id
			true // should queue overflowed
		};
	case ERPCType::ServerUnreliable:
		return {
			ERPCType::ServerUnreliable,
			SpatialConstants::CLIENT_ENDPOINT_COMPONENT_ID, // buffer comp
			SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::ServerUnreliable), // buffer size
			1 + SpatialGDKSettings->MaxRPCRingBufferSize + 1, // field start
			1 + SpatialGDKSettings->MaxRPCRingBufferSize + 1 + SpatialGDKSettings->MaxRPCRingBufferSize, // last sent field id
			SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID, // ack comp
			SpatialGDKSettings->MaxRPCRingBufferSize * 2 + 2 + 2, // ack field id
			false // should queue overflowed
		};
	case ERPCType::NetMulticast:
		return {
			ERPCType::NetMulticast,
			SpatialConstants::MULTICAST_RPCS_COMPONENT_ID, // buffer comp
			SpatialGDKSettings->GetRPCRingBufferSize(ERPCType::NetMulticast), // buffer size
			1, // field start
			1 + SpatialGDKSettings->MaxRPCRingBufferSize, // last sent field id
			SpatialConstants::INVALID_COMPONENT_ID, // ack comp
			0, // ack field id
			false // should queue overflowed
		};
	default:
		checkNoEntry();
		return RPCRingBufferDescriptor{};
	}
}

void ReadBufferFromSchema(Schema_Object* SchemaObject, const RPCRingBufferDescriptor& Descriptor, RPCRingBuffer& OutBuffer)
{
	for (uint32 RingBufferIndex = 0; RingBufferIndex < Descriptor.RingBufferSize; RingBufferIndex++)
	{
		Schema_FieldId FieldId = Descriptor.SchemaFieldStart + RingBufferIndex;
		if (Schema_GetObjectCount(SchemaObject, FieldId) > 0)
		{
			OutBuffer.RingBuffer[RingBufferIndex].Emplace(Schema_GetObject(SchemaObject, FieldId));
		}
	}

	if (Schema_GetUint64Count(SchemaObject, Descriptor.LastSentFieldId) > 0)
	{
		OutBuffer.LastSentRPCId = Schema_GetUint64(SchemaObject, Descriptor.LastSentFieldId);
	}
}

void ReadAckFromSchema(Schema_Object* SchemaObject, const RPCRingBufferDescriptor& Descriptor, uint64& OutAck)
{
	if (Schema_GetUint64Count(SchemaObject, Descriptor.AckFieldId) > 0)
	{
		OutAck = Schema_GetUint64(SchemaObject, Descriptor.AckFieldId);
	}
}

void WriteRPCToSchema(Schema_Object* SchemaObject, const RPCRingBufferDescriptor& Descriptor, uint64 RPCId, RPCPayload Payload)
{
	Schema_Object* RPCObject = Schema_AddObject(SchemaObject, Descriptor.GetRingBufferElementFieldId(RPCId));
	RPCPayload::WriteToSchemaObject(RPCObject, Payload.Offset, Payload.Index, Payload.PayloadData.GetData(), Payload.PayloadData.Num());

	Schema_ClearField(SchemaObject, Descriptor.LastSentFieldId);
	Schema_AddUint64(SchemaObject, Descriptor.LastSentFieldId, RPCId);
}

void WriteAckToSchema(Schema_Object* SchemaObject, const RPCRingBufferDescriptor& Descriptor, uint64 Ack)
{
	Schema_ClearField(SchemaObject, Descriptor.AckFieldId);
	Schema_AddUint64(SchemaObject, Descriptor.AckFieldId, Ack);
}

} // namespace RPCRingBufferUtils

} // namespace SpatialGDK
