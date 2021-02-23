// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialConstants.h"
#include "Utils/RPCRingBuffer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct CrossServerEndpoint
{
	CrossServerEndpoint(Schema_ComponentData* Data);

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update);

	RPCRingBuffer ReliableRPCBuffer;

private:
	void ReadFromSchema(Schema_Object* SchemaObject);
};

struct ACKItem
{
	void ReadFromSchema(Schema_Object* SchemaObject);
	void WriteToSchema(Schema_Object* SchemaObject);

	Worker_EntityId Sender = SpatialConstants::INVALID_ENTITY_ID;
	uint64 RPCId = 0;
	uint64 Result = 0;
};

struct CrossServerEndpointACK
{
	CrossServerEndpointACK(Schema_ComponentData* Data);

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update);

	// uint64 RPCAck = 0;
	TArray<TOptional<ACKItem>> ACKArray;

private:
	void ReadFromSchema(Schema_Object* SchemaObject);
};

} // namespace SpatialGDK
