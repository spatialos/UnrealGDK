// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialLegacyPartitionSystem.h"

USpatialLegacyPartitionSystem::USpatialLegacyPartitionSystem() {}

TArray<SpatialGDK::FLBDataStorage*> USpatialLegacyPartitionSystem::GetData()
{
	TArray<SpatialGDK::FLBDataStorage*> Storages;
	Storages.Add(&Cells);

	return Storages;
}
