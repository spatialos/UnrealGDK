// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "Schema/RPCRingBuffer.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct ServerRPCEndpointRB;

struct ClientRPCEndpointRB : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_RB;

	ClientRPCEndpointRB(const Worker_ComponentData& Data);

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) override;

	static Worker_ComponentData CreateRPCEndpointData();
	Worker_ComponentUpdate CreateRPCEndpointUpdate(const QueuedRPCMap* RPCMap, const TSet<ESchemaComponentType>* RPCTypesExecuted, const ServerRPCEndpointRB* ServerEndpoint);

	TArray<RPCPayload> RetrieveNewRPCs(ServerRPCEndpointRB& ServerEndpoint, TSet<ESchemaComponentType>& RPCTypesRetrieved);
	bool HasNewRPCs(const ServerRPCEndpointRB& ServerEndpoint) const;

	RPCRingBuffer ReliableRPCs;
	RPCRingBuffer UnreliableRPCs;
	uint64 LastExecutedReliableRPC = 0;
	uint64 LastExecutedUnreliableRPC = 0;
	uint64 LastExecutedMulticastRPC = 0;

	const Schema_FieldId LastExecutedReliableRPCFieldId;
	const Schema_FieldId LastExecutedUnreliableRPCFieldId;
	const Schema_FieldId LastExecutedMulticastRPCFieldId;
};

} // namespace SpatialGDK
