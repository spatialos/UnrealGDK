// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialInitialDormancyDeletionMap.h"

#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyTests/DormancyTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyTests/InitiallyDormantMapActorDeletionTest.h"

USpatialInitialDormancyDeletionMap::USpatialInitialDormancyDeletionMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE, TEXT("SpatialInitialDormancyDeletionMap"))
{
}

void USpatialInitialDormancyDeletionMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	AddActorToLevel<AInitiallyDormantMapActorDeletionTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ADormancyTestActor>(CurrentLevel, FTransform::Identity);
}
