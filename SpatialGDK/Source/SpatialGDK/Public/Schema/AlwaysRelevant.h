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
		Data.schema_type = Schema_CreateComponentData(ComponentId);

		return Data;
	}
};

} // namespace SpatialGDK
