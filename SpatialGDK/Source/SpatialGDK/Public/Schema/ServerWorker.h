// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include "Containers/UnrealString.h"

#include <WorkerSDK/improbable/c_schema.h>

namespace SpatialGDK
{

// The ServerWorker component exists to hold the physical worker name corresponding to a
// server worker entity. This is so that the translator can make virtual workers to physical
// worker names using the server worker entities.
struct ServerWorker : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::SERVER_WORKER_COMPONENT_ID;

	ServerWorker()
		: WorkerName(SpatialConstants::INVALID_WORKER_NAME)
		, bReadyToBeginPlay(false)
	{}

	ServerWorker(const PhysicalWorkerName& InWorkerName, const bool bInReadyToBeginPlay)
	{
		WorkerName = InWorkerName;
		bReadyToBeginPlay = bInReadyToBeginPlay;
	}

	ServerWorker(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		WorkerName = GetStringFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID);
		bReadyToBeginPlay = GetBoolFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID);
	}

	Worker_ComponentData CreateServerWorkerData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		AddStringToSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID, WorkerName);
		Schema_AddBool(ComponentObject, SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID, bReadyToBeginPlay);

		return Data;
	}

	Worker_ComponentUpdate CreateServerWorkerUpdate()
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		AddStringToSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID, WorkerName);
		Schema_AddBool(ComponentObject, SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID, bReadyToBeginPlay);

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		WorkerName = GetStringFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_NAME_ID);
		bReadyToBeginPlay = GetBoolFromSchema(ComponentObject, SpatialConstants::SERVER_WORKER_READY_TO_BEGIN_PLAY_ID);
	}

	PhysicalWorkerName WorkerName;
	bool bReadyToBeginPlay;
};

} // namespace SpatialGDK

