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

struct MulticastRPCEndpointRB : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::NETMULTICAST_RPCS_COMPONENT_ID_RB;

	MulticastRPCEndpointRB()
		: MulticastRPCs(GetDefault<USpatialGDKSettings>()->GetRPCRingBufferSize(SCHEMA_NetMulticastRPC), 1)
	{
	}

	MulticastRPCEndpointRB(const Worker_ComponentData& Data)
		: MulticastRPCEndpointRB()
	{
		Schema_Object* EndpointObject = Schema_GetComponentDataFields(Data.schema_type);
		MulticastRPCs.ReadFromSchema(EndpointObject);
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) override
	{
		Schema_Object* EndpointObject = Schema_GetComponentUpdateFields(Update.schema_type);
		MulticastRPCs.ReadFromSchema(EndpointObject);
	}

	Worker_ComponentData CreateRPCEndpointData(const QueuedRPCMap* RPCMap)
	{
		Worker_ComponentData Data{};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (RPCMap != nullptr)
		{
			if (const TArray<RPCPayload>* MulticastRPCArray = RPCMap->Find(SCHEMA_NetMulticastRPC))
			{
				MulticastRPCs.WriteToSchema(ComponentObject, *MulticastRPCArray);
			}
		}

		return Data;
	}

	// Create component update from RPCs and executed counters

	RPCRingBuffer MulticastRPCs;
};

} // namespace SpatialGDK
