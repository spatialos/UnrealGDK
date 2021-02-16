// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialPropertyReplicationMap.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPropertyReplication/SpatialTestPropertyReplication.h"

USpatialPropertyReplicationMap::USpatialPropertyReplicationMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE, TEXT("SpatialPropertyReplicationMap"))
{
}

void USpatialPropertyReplicationMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	AddActorToLevel<ASpatialTestPropertyReplication>(CurrentLevel, FTransform::Identity);
}
