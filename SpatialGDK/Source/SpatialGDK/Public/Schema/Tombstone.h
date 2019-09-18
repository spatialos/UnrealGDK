// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct Tombstone : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::TOMBSTONE_COMPONENT_ID;

	Tombstone() = default;

	Tombstone(const Worker_ComponentData& Data)
	{
		const Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (Schema_GetBoolCount(ComponentObject, SpatialConstants::TOMBSTONE_ISDEAD_ID) == 1)
		{
			bIsDead = GetBoolFromSchema(ComponentObject, SpatialConstants::TOMBSTONE_ISDEAD_ID);
		}
	}

	Worker_ComponentData CreateTombstoneData() const
	{
		Worker_ComponentData ComponentData = {};
		ComponentData.component_id = ComponentId;
		ComponentData.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(ComponentData.schema_type);

		Schema_AddBool(ComponentObject, SpatialConstants::TOMBSTONE_ISDEAD_ID, bIsDead);

		return ComponentData;
	}

	Worker_ComponentUpdate CreateTombstoneUpdate() const
	{
		Worker_ComponentUpdate ComponentUpdate = {};
		ComponentUpdate.component_id = ComponentId;
		ComponentUpdate.schema_type = Schema_CreateComponentUpdate(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(ComponentUpdate.schema_type);

		Schema_AddBool(ComponentObject, SpatialConstants::TOMBSTONE_ISDEAD_ID, bIsDead);

		return ComponentUpdate;
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		const Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);
		bIsDead = GetBoolFromSchema(ComponentObject, SpatialConstants::TOMBSTONE_ISDEAD_ID);
	}

	bool bIsDead;
};

} // namespace SpatialGDK
