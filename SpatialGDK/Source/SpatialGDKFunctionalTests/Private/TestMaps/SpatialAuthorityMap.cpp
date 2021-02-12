// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialAuthorityMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTestGameMode.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTestReplicatedActor.h"

USpatialAuthorityMap::USpatialAuthorityMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE, TEXT("SpatialAuthorityMap"))
{
}

void USpatialAuthorityMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// The actors were placed in one of the quadrants of the map, even though the map does not have multiworker
	FVector SpatialAuthorityTestActorPosition(-250, -250, 0);

	// Add the test
	ASpatialAuthorityTest* AuthTestActor =
		AddActorToLevel<ASpatialAuthorityTest>(CurrentLevel, FTransform(SpatialAuthorityTestActorPosition));

	// Add the helpers, as we need things placed in the level
	AuthTestActor->LevelActor = AddActorToLevel<ASpatialAuthorityTestActor>(CurrentLevel, FTransform(SpatialAuthorityTestActorPosition));
	AuthTestActor->LevelReplicatedActor =
		AddActorToLevel<ASpatialAuthorityTestReplicatedActor>(CurrentLevel, FTransform(SpatialAuthorityTestActorPosition));
	// Says "on the border", but this map doesn't have multi-worker...?
	AuthTestActor->LevelReplicatedActorOnBorder =
		AddActorToLevel<ASpatialAuthorityTestReplicatedActor>(CurrentLevel, FTransform(FVector(0, 0, 0)));

	AWorldSettings* WorldSettings = World->GetWorldSettings();
	WorldSettings->DefaultGameMode = ASpatialAuthorityTestGameMode::StaticClass();
}
