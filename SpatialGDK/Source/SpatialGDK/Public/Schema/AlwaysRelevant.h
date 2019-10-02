// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct AlwaysRelevant : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::ALWAYS_RELEVANT_COMPONENT_ID;

	AlwaysRelevant() = default;

	FORCEINLINE Worker_ComponentData CreateData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();

		return Data;
	}
};

struct Dormant : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::DORMANT_COMPONENT_ID;

	Dormant() = default;

	FORCEINLINE Worker_ComponentData CreateData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData();

		return Data;
	}
};

} // namespace SpatialGDK
