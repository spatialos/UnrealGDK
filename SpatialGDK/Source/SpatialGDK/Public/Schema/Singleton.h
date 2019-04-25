// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

struct Singleton : Component
{
	static const Worker_ComponentId ComponentId = SpatialConstants::SINGLETON_COMPONENT_ID;

	Singleton() = default;
	Singleton(const Worker_ComponentData& Data)
	{
	}

	FORCEINLINE Worker_ComponentData CreateSingletonData()
	{
		Worker_ComponentData Data = {};
		Data.component_id = ComponentId;
		Data.schema_type = Schema_CreateComponentData(ComponentId);

		return Data;
	}
};

} // namespace SpatialGDK
