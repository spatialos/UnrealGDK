// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialView/ComponentData.h"
#include "SpatialCommonTypes.h"

#include "Containers/Array.h"
#include "Containers/Map.h"
#include <improbable/c_worker.h>

namespace SpatialGDK
{

struct EntityViewElement
{
	TArray<ComponentData> Components;
	TArray<Worker_ComponentId> Authority;
};

using EntityView = TMap<Worker_EntityId_Key, EntityViewElement>;

} // namespace SpatialGDK
