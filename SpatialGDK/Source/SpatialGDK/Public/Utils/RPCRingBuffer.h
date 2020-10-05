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

	const TOptional<RPCPayload>& GetRingBufferElement(uint64 RPCId) const { return RingBuffer[(RPCId - 1) % RingBuffer.Num()]; }
	const TOptional<FUnrealObjectRef>& GetRingBufferCounterpart(uint64 RPCId) const { return Counterpart[(RPCId - 1) % RingBuffer.Num()]; }

	ERPCType Type;
	TArray<TOptional<RPCPayload>> RingBuffer;
	TArray<TOptional<FUnrealObjectRef>> Counterpart;
	uint64 LastSentRPCId = 0;
};

struct RPCRingBufferDescriptor
{
	uint32 GetRingBufferElementIndex(ERPCType Type, uint64 RPCId) const
	{
		if (Type == ERPCType::CrossServerSender || Type == ERPCType::CrossServerReceiver)
		{
			return ((RPCId - 1) % RingBufferSize) * 2;
		}
		return (RPCId - 1) % RingBufferSize;
	}

	Schema_FieldId GetRingBufferElementFieldId(ERPCType Type, uint64 RPCId) const
	{
		return SchemaFieldStart + GetRingBufferElementIndex(Type, RPCId);
	}

	uint32 RingBufferSize;
	Schema_FieldId SchemaFieldStart;
	Schema_FieldId LastSentRPCFieldId;
};

namespace RPCRingBufferUtils
{
Worker_ComponentId GetRingBufferComponentId(ERPCType Type);
RPCRingBufferDescriptor GetRingBufferDescriptor(ERPCType Type);
uint32 GetRingBufferSize(ERPCType Type);

Worker_ComponentId GetAckComponentId(ERPCType Type);

Schema_FieldId GetAckFieldId(ERPCType Type);

Schema_FieldId GetInitiallyPresentMulticastRPCsCountFieldId();

bool ShouldQueueOverflowed(ERPCType Type);

void ReadBufferFromSchema(Schema_Object* SchemaObject, RPCRingBuffer& OutBuffer);
void ReadAckFromSchema(const Schema_Object* SchemaObject, ERPCType Type, uint64& OutAck);

void WriteRPCToSchema(Schema_Object* SchemaObject, ERPCType Type, uint64 RPCId, const RPCPayload& Payload);
void WriteAckToSchema(Schema_Object* SchemaObject, ERPCType Type, uint64 Ack);
void WriteAckForSenderToSchema(Schema_Object* SchemaObject, ERPCType Type, TArray<FUnrealObjectRef> const& AckArray);

void MoveLastSentIdToInitiallyPresentCount(Schema_Object* SchemaObject, uint64 LastSentId);

} // namespace RPCRingBufferUtils

} // namespace SpatialGDK
