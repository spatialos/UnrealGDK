// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/RPCRingBuffer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct ServerEndpoint : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::SERVER_ENDPOINT_COMPONENT_ID;

	ServerEndpoint(const Worker_ComponentData& Data);

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) override;

	RPCRingBuffer ReliableRPCBuffer;
	RPCRingBuffer UnreliableRPCBuffer;
	uint64 ReliableRPCAck = 0;
	uint64 UnreliableRPCAck = 0;

private:
	void ReadFromSchema(Schema_Object* SchemaObject);
};

} // namespace SpatialGDK
