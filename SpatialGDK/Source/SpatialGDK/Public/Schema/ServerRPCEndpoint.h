// Copyright (c) Improbable Worlds Ltd, All Rights Reserved	

#pragma once	

#include "Schema/Component.h"	
#include "SpatialConstants.h"	
#include "Utils/SchemaUtils.h"	

#include <WorkerSDK/improbable/c_schema.h>	
#include <WorkerSDK/improbable/c_worker.h>	

namespace SpatialGDK
{

struct ServerRPCEndpoint : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::SERVER_RPC_ENDPOINT_COMPONENT_ID;

	ServerRPCEndpoint() = default;

	ServerRPCEndpoint(const Worker_ComponentData& Data)
	{
		Schema_Object* EndpointObject = Schema_GetComponentDataFields(Data.schema_type);
		bReady = GetBoolFromSchema(EndpointObject, SpatialConstants::UNREAL_RPC_ENDPOINT_READY_ID);
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* EndpointObject = Schema_GetComponentUpdateFields(Update.schema_type);
		if (Schema_GetBoolCount(EndpointObject, SpatialConstants::UNREAL_RPC_ENDPOINT_READY_ID) > 0)
		{
			bReady = GetBoolFromSchema(EndpointObject, SpatialConstants::UNREAL_RPC_ENDPOINT_READY_ID);
		}
	}

	Worker_ComponentData CreateRPCEndpointData()
	{
		Worker_ComponentData Data{};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);
		Schema_AddBool(ComponentObject, SpatialConstants::UNREAL_RPC_ENDPOINT_READY_ID, bReady);

		return Data;
	}

	Worker_ComponentUpdate CreateRPCEndpointUpdate()
	{
		Worker_ComponentUpdate Update{};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* UpdateObject = Schema_GetComponentUpdateFields(Update.schema_type);
		Schema_AddBool(UpdateObject, SpatialConstants::UNREAL_RPC_ENDPOINT_READY_ID, bReady);

		return Update;
	}

	bool bReady = false;
};

} // namespace SpatialGDK
