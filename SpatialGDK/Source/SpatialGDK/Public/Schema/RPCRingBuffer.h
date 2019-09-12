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
	RPCRingBuffer(uint32 InRingBufferSize, Schema_FieldId InSchemaFieldStart);

	void ReadFromSchema(Schema_Object* Data);

	// Always writes all the RPCs, even if need to overwrite.
	void WriteToSchema(Schema_Object* Data, const TArray<RPCPayload>& RPCs);

	// TODO: Get RPCs since given ID?
	TArray<RPCPayload> GetRPCsSince(uint64 LastExecutedRPCId);

	inline Schema_FieldId GetLastSentRPCIdFieldId()
	{
		return SchemaFieldStart + RingBufferSize;
	}

	int32 GetCapacity(uint64 LastExecutedRPCId)
	{
		return LastExecutedRPCId + RingBufferSize - LastSentRPCId;
	}

	// Passed in constructor, can't change.
	const uint32 RingBufferSize;
	const Schema_FieldId SchemaFieldStart;

	TArray<TOptional<RPCPayload>> RingBuffer;

	uint64 LastSentRPCId = 0;
};

} // namespace SpatialGDK
