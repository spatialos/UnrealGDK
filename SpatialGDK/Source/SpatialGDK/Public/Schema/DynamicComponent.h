// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"

#include <WorkerSDK/improbable/c_worker.h>

namespace SpatialGDK
{

// Represents any Unreal rep component
struct DynamicComponent : Component
{
	DynamicComponent() = default;

	DynamicComponent(const Worker_ComponentData& InComponentData)
		: ComponentData(Worker_AcquireComponentData(&InComponentData))
	{
	}

	~DynamicComponent()
	{
		Worker_ReleaseComponentData(ComponentData);
	}

	Worker_ComponentData* ComponentData;
};

} // namespace SpatialGDK
