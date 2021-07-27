// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialServerWorkerSystemImpl.h"
#include "SpatialConstants.h"

namespace SpatialGDK
{
void FServerWorkerSystemImpl::Flush(Worker_EntityId ServerWorkerEntityId, ISpatialOSWorker& Connection)
{
	for (auto& Update : PendingComponentUpdates)
	{
		Connection.SendComponentUpdate(ServerWorkerEntityId, MoveTemp(Update));
	}
	PendingComponentUpdates.Empty();
}
} // namespace SpatialGDK
