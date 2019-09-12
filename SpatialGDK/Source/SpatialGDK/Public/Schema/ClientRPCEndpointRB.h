// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "Schema/RPCPayload.h"
#include "Schema/RPCRingBuffer.h"
#include "SpatialConstants.h"
#include "SpatialGDKSettings.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct ClientRPCEndpointRB : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID_RB;

	ClientRPCEndpointRB(const Worker_ComponentData& Data)
		: ReliableRPCs(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(SCHEMA_ServerReliableRPC), 1)
		, UnreliableRPCs(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(SCHEMA_ServerUnreliableRPC), ReliableRPCs.SchemaFieldStart + ReliableRPCs.RingBufferSize)
		, LastExecutedReliableRPCFieldId(UnreliableRPCs.SchemaFieldStart + UnreliableRPCs.RingBufferSize)
		, LastExecutedUnreliableRPCFieldId(LastExecutedReliableRPCFieldId + 1)
		, LastExecutedMulticastRPCFieldId(LastExecutedUnreliableRPCFieldId + 1)
	{
		Schema_Object* EndpointObject = Schema_GetComponentDataFields(Data.schema_type);
		ReliableRPCs.ReadFromSchema(EndpointObject);
		UnreliableRPCs.ReadFromSchema(EndpointObject);

		LastExecutedReliableRPC = Schema_GetUint64(EndpointObject, LastExecutedReliableRPCFieldId);
		LastExecutedUnreliableRPC = Schema_GetUint64(EndpointObject, LastExecutedUnreliableRPCFieldId);
		LastExecutedMulticastRPC = Schema_GetUint64(EndpointObject, LastExecutedMulticastRPCFieldId);
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) override
	{
		Schema_Object* EndpointObject = Schema_GetComponentUpdateFields(Update.schema_type);
		ReliableRPCs.ReadFromSchema(EndpointObject);
		UnreliableRPCs.ReadFromSchema(EndpointObject);

		if (Schema_GetUint64Count(EndpointObject, LastExecutedReliableRPCFieldId) > 0)
		{
			LastExecutedReliableRPC = Schema_GetUint64(EndpointObject, LastExecutedReliableRPCFieldId);
		}
		if (Schema_GetUint64Count(EndpointObject, LastExecutedUnreliableRPCFieldId) > 0)
		{
			LastExecutedUnreliableRPC = Schema_GetUint64(EndpointObject, LastExecutedUnreliableRPCFieldId);
		}
		if (Schema_GetUint64Count(EndpointObject, LastExecutedMulticastRPCFieldId) > 0)
		{
			LastExecutedMulticastRPC = Schema_GetUint64(EndpointObject, LastExecutedMulticastRPCFieldId);
		}
	}

	static Worker_ComponentData CreateRPCEndpointData()
	{
		Worker_ComponentData Data{};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		return Data;
	}

	// Create component update from RPCs and executed counters

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
