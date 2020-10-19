// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/RPCRingBuffer.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct MulticastRPCs : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::MULTICAST_RPCS_COMPONENT_ID;

	MulticastRPCs(const Worker_ComponentData& Data);

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) override;

	RPCRingBuffer MulticastRPCBuffer;
	uint32 InitiallyPresentMulticastRPCsCount = 0;

private:
	void ReadFromSchema(Schema_Object* SchemaObject);
};

} // namespace SpatialGDK
