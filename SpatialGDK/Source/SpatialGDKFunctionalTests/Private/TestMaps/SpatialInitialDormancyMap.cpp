// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialInitialDormancyMap.h"

#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyTests/DormancyTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyTests/InitiallyDormantMapActorTest.h"

USpatialInitialDormancyMap::USpatialInitialDormancyMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE, TEXT("SpatialInitialDormancyMap"))
{
}

void USpatialInitialDormancyMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	AddActorToLevel<AInitiallyDormantMapActorTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ADormancyTestActor>(CurrentLevel, FTransform::Identity);
}
