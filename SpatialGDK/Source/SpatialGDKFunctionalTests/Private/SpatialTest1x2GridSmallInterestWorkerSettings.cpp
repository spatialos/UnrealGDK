// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTest1x2GridSmallInterestWorkerSettings.h"

#include "SpatialTest1x2GridSmallInterestStrategy.h"

USpatialTest1x2GridSmallInterestWorkerSettings::USpatialTest1x2GridSmallInterestWorkerSettings()
{
	WorkerLayers[0].LoadBalanceStrategy = USpatialTest1x2GridSmallInterestStrategy::StaticClass();
}
