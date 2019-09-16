// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "Schema/RPCRingBuffer.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct ClientRPCEndpointRB;

struct ServerRPCEndpointRB : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID_RB;

	ServerRPCEndpointRB();
	ServerRPCEndpointRB(const Worker_ComponentData& Data);

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) override;

	Worker_ComponentData CreateRPCEndpointData(const QueuedRPCMap* RPCMap);
	Worker_ComponentUpdate CreateRPCEndpointUpdate(const QueuedRPCMap* RPCMap, const TSet<ESchemaComponentType>* RPCTypesExecuted, const ClientRPCEndpointRB* ClientEndpoint);

	TArray<RPCPayload> RetrieveNewRPCs(ClientRPCEndpointRB& ClientEndpoint, TSet<ESchemaComponentType>& RPCTypesRetrieved);
	bool HasNewRPCs(const ClientRPCEndpointRB& ClientEndpoint) const;

	// Create component update from RPCs and executed counters

	RPCRingBuffer ReliableRPCs;
	RPCRingBuffer UnreliableRPCs;
	uint64 LastExecutedReliableRPC = 0;
	uint64 LastExecutedUnreliableRPC = 0;

	const Schema_FieldId LastExecutedReliableRPCFieldId;
	const Schema_FieldId LastExecutedUnreliableRPCFieldId;
};

} // namespace SpatialGDK
