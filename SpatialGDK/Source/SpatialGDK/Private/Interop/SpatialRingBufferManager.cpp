// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialRingBufferManager.h"

#include "EngineClasses/SpatialNetDriver.h"

void USpatialRingBufferManager::Init(USpatialNetDriver* InNetDriver)
{
	NetDriver = InNetDriver;
	Sender = NetDriver->Sender;
	StaticComponentView = NetDriver->StaticComponentView;
}


