// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "Schema/RPCRingBuffer.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct MulticastRPCEndpointRB : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_RB;

	MulticastRPCEndpointRB();
	MulticastRPCEndpointRB(const Worker_ComponentData& Data);

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) override;

	Worker_ComponentData CreateRPCEndpointData(const QueuedRPCMap* RPCMap);
	Worker_ComponentUpdate CreateRPCEndpointUpdate(const QueuedRPCMap* RPCMap);

	TArray<RPCPayload> RetrieveNewRPCs();

	RPCRingBuffer MulticastRPCs;
	uint64 LastExecutedMulticastRPC = 0;
};

} // namespace SpatialGDK
