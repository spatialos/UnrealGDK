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
	SpatialStrategySystem(const FSubView& InSubView, Worker_EntityId InStrategyWorkerEntityId);
	void Advance();
	void Flush();
	void Destroy();

private:
	const FSubView& SubView;
	Worker_EntityId StrategyWorkerEntityId;
	Worker_EntityId StrategyPartitionEntityId;
};
} // namespace SpatialGDK
