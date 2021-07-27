// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialServerWorkerSystem.h"
#include "LoadBalancing/LBDataStorage.h"

namespace SpatialGDK
{
class ISpatialOSWorker;

class FServerWorkerSystemImpl
{
public:
	void Flush(Worker_EntityId ServerWorkerEntityId, ISpatialOSWorker& Connection);

	TSet<Worker_ComponentId> ServerWorkerComponents;
	TArray<SpatialGDK::ComponentUpdate> PendingComponentUpdates;
};
} // namespace SpatialGDK
