// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialServerWorkerSystem.h"
#include "LoadBalancing/LBDataStorage.h"

namespace SpatialGDK
{
class FServerWorkerSystemImpl
{
public:
	TSet<Worker_ComponentId> ServerWorkerComponents;
	TArray<SpatialGDK::ComponentUpdate> PendingComponentUpdates;
};
} // namespace SpatialGDK
