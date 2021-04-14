// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct PlayerControllerServer : AbstractMutableComponent
{
	static const Worker_ComponentId ComponentId = SpatialConstants::PLAYER_CONTROLLER_SERVER_COMPONENT_ID;

	PlayerControllerServer() = default;

	PlayerControllerServer(const Worker_EntityId& InAuthServerWorkerEntityID) { AuthServerWorkerEntityID = InAuthServerWorkerEntityID; }

	explicit PlayerControllerServer(const Worker_ComponentData& Data)
		: PlayerControllerServer(Data.schema_type)
	{
	}

	explicit PlayerControllerServer(Schema_ComponentData* Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);

		AuthServerWorkerEntityID = Schema_GetEntityId(ComponentObject, 1);
	}

	Worker_ComponentData CreateComponentData() const override
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Schema_AddEntityId(ComponentObject, 1, AuthServerWorkerEntityID);

		return Data;
	}

	Worker_ComponentUpdate CreateComponentUpdate()
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		Schema_AddEntityId(ComponentObject, 1, AuthServerWorkerEntityID);

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update) { ApplyComponentUpdate(Update.schema_type); }

	void ApplyComponentUpdate(Schema_ComponentUpdate* Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update);

		AuthServerWorkerEntityID = Schema_GetEntityId(ComponentObject, 1);
	}

	Worker_EntityId AuthServerWorkerEntityID;
};

} // namespace SpatialGDK
