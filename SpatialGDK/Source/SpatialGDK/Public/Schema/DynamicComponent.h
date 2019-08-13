// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"

#include <WorkerSDK/improbable/c_worker.h>
#include <memory>

namespace SpatialGDK
{

// Represents any Unreal rep component
struct DynamicComponent : Component
{
	DynamicComponent() = default;

	DynamicComponent(const Worker_ComponentData& InComponentData)
		: ComponentData(new Worker_ComponentData{ InComponentData })
	{
		ComponentData->schema_type = Schema_CopyComponentData(InComponentData.schema_type);
	}

	struct Deleter
	{
		void operator()(Worker_ComponentData* Data) const noexcept
		{
			Schema_DestroyComponentData(Data->schema_type);
		}
	};

	std::unique_ptr<Worker_ComponentData, Deleter> ComponentData;
};

} // namespace SpatialGDK
