// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Schema/CrossServerEndpoint.h"
#include "SpatialView/SubView.h"
#include "Utils/CrossServerUtils.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialStrategySystem, Log, All)

class SpatialOSWorkerInterface;

namespace SpatialGDK
{
class SpatialStrategySystem
{
public:
	SpatialStrategySystem(const FSubView& InSubView, Worker_EntityId InStrategyWorkerSystemEntityId, SpatialOSWorkerInterface* Connection);

	void Advance(SpatialOSWorkerInterface* Connection);
	void Flush(SpatialOSWorkerInterface* Connection);
	void Destroy(SpatialOSWorkerInterface* Connection);

private:
	const FSubView& SubView;

	void CreateStrategyPartition(SpatialOSWorkerInterface* Connection);
	TOptional<Worker_RequestId> StrategyWorkerRequest;
	Worker_EntityId StrategyPartition;
	Worker_EntityId StrategyWorkerSystemEntityId;
};
} // namespace SpatialGDK
