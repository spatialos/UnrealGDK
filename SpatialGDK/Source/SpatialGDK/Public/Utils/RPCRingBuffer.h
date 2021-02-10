// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Misc/Optional.h"

#include "Schema/RPCComponentLayout.h"
#include "Schema/RPCPayload.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct RPCRingBuffer
{
	RPCRingBuffer(ERPCType InType);

	const TOptional<RPCPayload>& GetRingBufferElement(uint64 RPCId) const { return RingBuffer[(RPCId - 1) % RingBuffer.Num()]; }

	ERPCType Type;
	TArray<TOptional<RPCPayload>> RingBuffer;
	uint64 LastSentRPCId = 0;
};

struct RPCRingBufferDescriptor
{
	RPCRingBufferDescriptor() = default;
	RPCRingBufferDescriptor(Schema_FieldId InSchemaFieldStart, uint32 InBufferSize)
		: RingBufferSize(InBufferSize)
		, SchemaFieldStart(InSchemaFieldStart)
		, LastSentRPCFieldId(SchemaFieldStart + RingBufferSize)
	{
	}

	uint32 GetRingBufferElementIndex(uint64 RPCId) const { return (RPCId - 1) % RingBufferSize; }

	Schema_FieldId GetRingBufferElementFieldId(uint64 RPCId) const { return SchemaFieldStart + GetRingBufferElementIndex(RPCId); }

	Schema_FieldId GetSchemaFieldEnd() const { return LastSentRPCFieldId; }

	uint32 RingBufferSize = 0;
	Schema_FieldId SchemaFieldStart = 0;
	Schema_FieldId LastSentRPCFieldId = 0;
};

namespace RPCRingBufferUtils
{
Worker_ComponentId GetRingBufferComponentId(ERPCType Type);
Worker_ComponentId GetRingBufferAuthComponentSetId(ERPCType Type);
uint32 GetRingBufferSize(ERPCType Type);

Worker_ComponentId GetAckComponentId(ERPCType Type);
Worker_ComponentId GetAckAuthComponentSetId(ERPCType Type);

Schema_FieldId GetInitiallyPresentMulticastRPCsCountFieldId();

bool ShouldQueueOverflowed(ERPCType Type);
bool ShouldIgnoreCapacity(ERPCType Type);

} // namespace RPCRingBufferUtils

class RPCComponentLayout
{
public:
	void ReadBufferFromSchema(Schema_Object* SchemaObject, RPCRingBuffer& OutBuffer);
	void ReadAckFromSchema(const Schema_Object* SchemaObject, ERPCType Type, uint64& OutAck);

	void WriteRPCToSchema(Schema_Object* SchemaObject, ERPCType Type, uint64 RPCId, const RPCPayload& Payload);
	void WriteAckToSchema(Schema_Object* SchemaObject, ERPCType Type, uint64 Ack);

	virtual RPCRingBufferDescriptor GetRingBufferDescriptor(ERPCType Type) const { return RPCRingBufferDescriptor(); }
	virtual Schema_FieldId GetAckFieldId(ERPCType Type) const { return 0; }
};

} // namespace SpatialGDK
