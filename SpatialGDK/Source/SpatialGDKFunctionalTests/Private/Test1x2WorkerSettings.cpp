// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Test1x2WorkerSettings.h"

#include "Test1x2GridStrategy.h"

UTest1x2WorkerSettings::UTest1x2WorkerSettings()
{
	WorkerLayers[0].LoadBalanceStrategy = UTest1x2GridStrategy::StaticClass();
}
