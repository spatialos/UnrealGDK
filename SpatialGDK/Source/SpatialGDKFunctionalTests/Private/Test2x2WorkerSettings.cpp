// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Test2x2WorkerSettings.h"

#include "Test2x2GridStrategy.h"

UTest2x2WorkerSettings::UTest2x2WorkerSettings()
{
	WorkerLayers[0].LoadBalanceStrategy = UTest2x2GridStrategy::StaticClass();
}
