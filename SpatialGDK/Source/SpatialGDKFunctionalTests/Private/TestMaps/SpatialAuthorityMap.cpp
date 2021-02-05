// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialAuthorityMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTestGameMode.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTestReplicatedActor.h"

USpatialAuthorityMap::USpatialAuthorityMap()
{
	MapCategory = CI_FAST;
	MapName = TEXT("SpatialAuthorityMap");
}

void USpatialAuthorityMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the test
	ASpatialAuthorityTest* AuthTestActor = CastChecked<ASpatialAuthorityTest>(
		GEditor->AddActor(CurrentLevel, ASpatialAuthorityTest::StaticClass(), FTransform(FVector(-250, -250, 0))));

	// Add the helpers, as we need things placed in the level
	AuthTestActor->LevelActor = CastChecked<ASpatialAuthorityTestActor>(
		GEditor->AddActor(CurrentLevel, ASpatialAuthorityTestActor::StaticClass(), FTransform(FVector(-250, -250, 0))));
	AuthTestActor->LevelReplicatedActor = CastChecked<ASpatialAuthorityTestReplicatedActor>(
		GEditor->AddActor(CurrentLevel, ASpatialAuthorityTestReplicatedActor::StaticClass(), FTransform(FVector(-250, -250, 0))));
	AuthTestActor->LevelReplicatedActorOnBorder = CastChecked<ASpatialAuthorityTestReplicatedActor>(
		GEditor->AddActor(CurrentLevel, ASpatialAuthorityTestReplicatedActor::StaticClass(),
						  FTransform(FVector(0, 0, 0)))); // Says "on the border", but this map doesn't have multi-worker...?

	AWorldSettings* WorldSettings = World->GetWorldSettings();
	WorldSettings->DefaultGameMode = ASpatialAuthorityTestGameMode::StaticClass();
}
