// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/Actor.h"
#include "Interop/SpatialClassInfoManager.h"
#include "Schema/Component.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"
#include "UObject/Package.h"
#include "UObject/UObjectHash.h"
#include "Utils/SchemaUtils.h"

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
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		if (Schema_GetBoolCount(ComponentObject, 1) == 1)
		{
			bIsDead = GetBoolFromSchema(ComponentObject, SpatialConstants::TOMBSTONE_ISDEAD_ID);
		}
	}

	Worker_ComponentData CreateTombstoneData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data.schema_type);

		Schema_AddBool(ComponentObject, SpatialConstants::TOMBSTONE_ISDEAD_ID, bIsDead);

		return Data;
	}

	Worker_ComponentUpdate CreateTombstoneUpdate()
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
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);
		bIsDead = GetBoolFromSchema(ComponentObject, SpatialConstants::TOMBSTONE_ISDEAD_ID);
	}

	bool bIsDead;
};

} // namespace SpatialGDK
