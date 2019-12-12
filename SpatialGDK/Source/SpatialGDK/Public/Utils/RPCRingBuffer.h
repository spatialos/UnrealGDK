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
	RPCRingBuffer(ERPCType InType);

	const TOptional<RPCPayload>& GetRingBufferElement(uint64 RPCId) const
	{
		return RingBuffer[(RPCId - 1) % RingBuffer.Num()];
	}

	ERPCType Type;
	TArray<TOptional<RPCPayload>> RingBuffer;
	uint64 LastSentRPCId = 0;
};

struct RPCRingBufferDescriptor
{
	uint32 GetRingBufferElementIndex(uint64 RPCId) const
	{
		return (RPCId - 1) % RingBufferSize;
	}

	Schema_FieldId GetRingBufferElementFieldId(uint64 RPCId) const
	{
		return SchemaFieldStart + GetRingBufferElementIndex(RPCId);
	}

	uint32 RingBufferSize;
	Schema_FieldId SchemaFieldStart;
	Schema_FieldId LastSentRPCFieldId;
};

namespace RPCRingBufferUtils
{

SPATIALGDK_API Worker_ComponentId GetRingBufferComponentId(ERPCType Type);
RPCRingBufferDescriptor GetRingBufferDescriptor(ERPCType Type);
uint32 GetRingBufferSize(ERPCType Type);

Worker_ComponentId GetAckComponentId(ERPCType Type);
Schema_FieldId GetAckFieldId(ERPCType Type);

Schema_FieldId GetInitiallyPresentMulticastRPCsCountFieldId();

bool ShouldQueueOverflowed(ERPCType Type);

void ReadBufferFromSchema(Schema_Object* SchemaObject, RPCRingBuffer& OutBuffer);
void ReadAckFromSchema(Schema_Object* SchemaObject, ERPCType Type, uint64& OutAck);

SPATIALGDK_API void WriteRPCToSchema(Schema_Object* SchemaObject, ERPCType Type, uint64 RPCId, const RPCPayload& Payload);
SPATIALGDK_API void WriteAckToSchema(Schema_Object* SchemaObject, ERPCType Type, uint64 Ack);

void MoveLastSentIdToInitiallyPresentCount(Schema_Object* SchemaObject, uint64 LastSentId);

} // namespace RPCRingBufferUtils

} // namespace SpatialGDK
