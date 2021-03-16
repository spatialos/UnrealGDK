// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>

namespace SpatialGDK
{
// The SpatialDebugging component exists to hold information which needs to be displayed by the
// SpatialDebugger on clients but which would not normally be available to clients.
struct CustomPersistence : AbstractMutableComponent
{
	static const Worker_ComponentId ComponentId = SpatialConstants::CUSTOM_PERSISTENCE_ID;

	CustomPersistence()
		: Depleted(false)
	{
	}

	CustomPersistence(const bool DepletedIn)
	{
		Depleted = DepletedIn;
	}

	CustomPersistence(const Worker_ComponentData& Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Depleted = Schema_GetBool(ComponentObject, SpatialConstants::CUSTOM_PERSISTENCE_DEPLETED) != 0;
	}

	Worker_ComponentData CreateComponentData() const override
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Schema_AddBool(ComponentObject, SpatialConstants::CUSTOM_PERSISTENCE_DEPLETED, Depleted);

		return Data;
	}

	Worker_ComponentUpdate CreateSpatialDebuggingUpdate()
	{
		Worker_ComponentUpdate Update = {};
		Update.component_id = ComponentId;
		Update.schema_type = Schema_CreateComponentUpdate();
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		Schema_AddBool(ComponentObject, SpatialConstants::CUSTOM_PERSISTENCE_DEPLETED, Depleted);

		return Update;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		Depleted = Schema_GetBool(ComponentObject, SpatialConstants::CUSTOM_PERSISTENCE_DEPLETED) != 0;
	}

	bool Depleted;
};

} // namespace SpatialGDK
