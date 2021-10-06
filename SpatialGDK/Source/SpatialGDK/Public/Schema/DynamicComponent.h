// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"
#include "SpatialView/ComponentData.h"

namespace SpatialGDK
{
// Represents any Unreal rep component
struct DynamicComponent : Component
{
	explicit DynamicComponent(const Worker_ComponentData& InComponentData)
		: Data(ComponentData::CreateCopy(InComponentData.schema_type, InComponentData.component_id))
	{
	}

	ComponentData Data;
};

} // namespace SpatialGDK
