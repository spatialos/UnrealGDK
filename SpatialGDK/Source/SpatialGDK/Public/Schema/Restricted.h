// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialCommonTypes.h"
#include "Utils/SchemaUtils.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{
struct Partition : Component
{
	static constexpr Worker_ComponentId ComponentId = SpatialConstants::PARTITION_COMPONENT_ID;

	Partition() = default;

	Partition(const Worker_ComponentData& Data)
		: Partition(Data.schema_type)
	{
	}

	Partition(Schema_ComponentData* Data)
	{
		Schema_Object* ComponentObject = Schema_GetComponentDataFields(Data);

		WorkerConnectionId = Schema_GetUint64(ComponentObject, 1);
	}

	void ApplyComponentUpdate(const Worker_ComponentUpdate& Update)
	{
		Schema_Object* ComponentObject = Schema_GetComponentUpdateFields(Update.schema_type);

		WorkerConnectionId = Schema_GetUint64(ComponentObject, 1);
	}

	Worker_EntityId_Key WorkerConnectionId;
};

} // namespace SpatialGDK
