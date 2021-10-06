// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Map.h"
#include "Containers/Set.h"
#include "improbable/c_worker.h"

namespace SpatialGDK
{
struct FComponentSetData
{
	TMap<Worker_ComponentSetId, TSet<Worker_ComponentId>> ComponentSets;
};
} // namespace SpatialGDK
