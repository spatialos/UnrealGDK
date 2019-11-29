// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Misc/Optional.h"

#include "Schema/RPCPayload.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct RPCRingBuffer
{
	TArray<TOptional<RPCPayload>> RingBuffer;
	uint64 LastSentRPCId = 0;
};

struct RPCRingBufferDescriptor
{
	inline uint32 GetRingBufferElementIndex(uint64 RPCId) const
	{
		return (RPCId - 1) % RingBufferSize;
	}

	inline Schema_FieldId GetRingBufferElementFieldId(uint64 RPCId) const
	{
		return SchemaFieldStart + GetRingBufferElementIndex(RPCId);
	}

	inline bool HasCapacity(uint64 LastAckedRPCId, uint64 NewRPCId) const
	{
		return LastAckedRPCId + RingBufferSize >= NewRPCId;
	}

	ERPCType Type;

	Worker_ComponentId RingBufferComponentId;
	uint32 RingBufferSize;
	Schema_FieldId SchemaFieldStart;
	Schema_FieldId LastSentFieldId;

	Worker_ComponentId AckComponentId;
	Schema_FieldId AckFieldId;

	bool bShouldQueueOverflowed;
};

namespace RPCRingBufferUtils
{

RPCRingBufferDescriptor GetRingBufferDescriptor(ERPCType Type);

void ReadBufferFromSchema(Schema_Object* SchemaObject, const RPCRingBufferDescriptor& Descriptor, RPCRingBuffer& OutBuffer);
void ReadAckFromSchema(Schema_Object* SchemaObject, const RPCRingBufferDescriptor& Descriptor, uint64& OutAck);

void WriteRPCToSchema(Schema_Object* SchemaObject, const RPCRingBufferDescriptor& Descriptor, uint64 RPCId, RPCPayload Payload);
void WriteAckToSchema(Schema_Object* SchemaObject, const RPCRingBufferDescriptor& Descriptor, uint64 Ack);

} // namespace RPCRingBufferUtils

} // namespace SpatialGDK
