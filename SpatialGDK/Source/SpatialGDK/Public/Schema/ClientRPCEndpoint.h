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

	ClientRPCEndpoint(bool InClientProcessedRPCsOnEntityCreation = false) : ClientProcessedRPCsOnEntityCreation(InClientProcessedRPCsOnEntityCreation)
	{}

	ClientRPCEndpoint(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
		ClientProcessedRPCsOnEntityCreation = GetBoolFromSchema(ComponentObject, SpatialConstants::CLIENT_PROCESSED_RPCS_ON_ENTITY_CREATION);
	}

	Worker_ComponentData CreateClientRPCEndpointData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
		Schema_AddBool(ComponentObject, SpatialConstants::CLIENT_PROCESSED_RPCS_ON_ENTITY_CREATION, ClientProcessedRPCsOnEntityCreation);

		return Data;
	}

	Worker_ComponentUpdate CreateClientRPCEndpointUpdate()
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID;
		Update.schema_type = Schema_CreateComponentUpdate(SpatialConstants::CLIENT_RPC_ENDPOINT_COMPONENT_ID);
		Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);
		Schema_AddBool(UpdateObject, SpatialConstants::CLIENT_PROCESSED_RPCS_ON_ENTITY_CREATION, ClientProcessedRPCsOnEntityCreation);

		return Update;
	}

	bool ClientProcessedRPCsOnEntityCreation;
};

} // namespace SpatialGDK
