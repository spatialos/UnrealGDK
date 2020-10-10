// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Interop/SpatialStaticComponentView.h"

#include <WorkerSDK/improbable/c_worker.h>

struct SPATIALGDK_API TestingComponentViewHelpers
{
	// Can be used add components to a component view for a given entity.
	static void AddEntityComponentToStaticComponentView(USpatialStaticComponentView& StaticComponentView, const FEntityId EntityId,
														const FComponentId ComponentId, Schema_ComponentData* ComponentData,
														const Worker_Authority Authority);

	static void AddEntityComponentToStaticComponentView(USpatialStaticComponentView& StaticComponentView, const FEntityId EntityId,
														const FComponentId ComponentId, const Worker_Authority Authority);
};
