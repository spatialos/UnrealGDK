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
	TArray<Worker_ComponentSetId> Authority;
};

using EntityView = TMap<Worker_EntityId_Key, EntityViewElement>;

} // namespace SpatialGDK
