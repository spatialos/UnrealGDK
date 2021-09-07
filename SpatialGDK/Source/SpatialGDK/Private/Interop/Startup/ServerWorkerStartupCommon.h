#pragma once

#include "Containers/Array.h"
#include "Misc/Optional.h"
#include "Templates/SharedPointer.h"

#include "SpatialCommonTypes.h"

namespace SpatialGDK
{
struct FServerWorkerStartupContext : public TSharedFromThis<FServerWorkerStartupContext>
{
	TOptional<Worker_EntityId> WorkerEntityId;

	TArray<Worker_EntityId> WorkerEntityIds;

	bool bHasGSMAuth = false;

	bool bIsRecoveringOrSnapshot = false;
};
} // namespace SpatialGDK
