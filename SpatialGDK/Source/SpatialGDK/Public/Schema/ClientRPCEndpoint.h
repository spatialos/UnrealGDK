// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct ClientRPCEndpoint : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID;

	ClientRPCEndpoint() = default;

	static Worker_ComponentData CreateClientRPCEndpointData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		return Data;
	}
};

} // namespace SpatialGDK
