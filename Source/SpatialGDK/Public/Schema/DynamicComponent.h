#pragma once

#include "CoreTypes/Component.h"
#include "Utils/SchemaUtils.h"
#include "Platform.h"

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>

// Represents any Unreal rep component
struct DynamicComponent : Component
{
	DynamicComponent()
	{
		bIsDynamic = true;
	}
	
	DynamicComponent(const Worker_ComponentData& InData)
		: Data(Worker_AcquireComponentData(&InData))
	{
		bIsDynamic = true;
	}

	~DynamicComponent()
	{
		Worker_ReleaseComponentData(Data);
	}

	Worker_ComponentData* Data;
};

