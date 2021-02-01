// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialConstants.h"
#include "Utils/RPCRingBuffer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct ServerEndpoint
{
	ServerEndpoint(Schema_ComponentData* Data);

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update);

	RPCRingBuffer ReliableRPCBuffer;
	RPCRingBuffer UnreliableRPCBuffer;
	uint64 ReliableRPCAck = 0;
	uint64 UnreliableRPCAck = 0;
	uint64 AlwaysWriteRPCAck = 0;

private:
	void ReadFromSchema(Schema_Object* SchemaObject);
};

} // namespace SpatialGDK
