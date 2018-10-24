// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Schema/Component.h"

#include <WorkerSDK/improbable/c_worker.h>

namespace improbable
{

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

}
