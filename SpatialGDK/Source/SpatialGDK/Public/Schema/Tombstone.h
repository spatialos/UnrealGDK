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

	FORCEINLINE Worker_ComponentData CreateData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();

		return Data;
	}
};

} // namespace SpatialGDK
