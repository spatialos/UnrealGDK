// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include "Containers/UnrealString.h"

#include <WorkerSDK/improbable/c_schema.h>

namespace SpatialGDK
{

// The ServerWorker component exists to hold information which is used to
// ensure non-GSM-authoritative workers correctly wait to know how they should deal with
// startup Actors
struct ServerWorker : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::SERVER_WORKER_COMPONENT_ID;

	ServerWorker()
		: WorkerName(SpatialConstants::INVALID_WORKER_NAME)
	{}

	ServerWorker(const PhysicalWorkerName& InWorkerName)
	{
		WorkerName = InWorkerName;
	}

	ServerWorker(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		WorkerName = GetStringFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID);
	}

	Worker_ComponentData CreateServerWorkerData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddStringToSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID, WorkerName);

		return Data;
	}

	Worker_ComponentUpdate CreateServerWorkerUpdate()
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		AddStringToSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID, WorkerName);

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		WorkerName = GetStringFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID);
	}

	PhysicalWorkerName WorkerName;
};

} // namespace SpatialGDK

