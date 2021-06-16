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
	SpatialStrategySystem(const FSubView& InSubView, FSpatialEntityId InStrategyWorkerEntityId, SpatialOSWorkerInterface* Connection);
	void Advance(SpatialOSWorkerInterface* Connection);
	void Flush(SpatialOSWorkerInterface* Connection);
	void Destroy(SpatialOSWorkerInterface* Connection);

private:
	const FSubView& SubView;
	FSpatialEntityId StrategyWorkerEntityId;
	FSpatialEntityId StrategyPartitionEntityId;
	Worker_RequestId StrategyWorkerRequest;
};
} // namespace SpatialGDK
