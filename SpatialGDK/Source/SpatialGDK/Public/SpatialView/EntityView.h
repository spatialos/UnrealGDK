// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialCommonTypes.h"
#include "SpatialView/ComponentData.h"

#include "Containers/Array.h"
#include "Containers/Map.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{
struct EntityViewElement
{
	TArray<ComponentData> Components;
	TArray<FComponentId> Authority;
};

using EntityView = TMap<FEntityId, EntityViewElement>;

} // namespace SpatialGDK
