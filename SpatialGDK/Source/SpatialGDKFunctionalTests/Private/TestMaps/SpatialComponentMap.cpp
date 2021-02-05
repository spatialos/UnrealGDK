// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialComponentMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/Public/Test1x2WorkerSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTestGameMode.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialComponentTest/SpatialComponentTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialComponentTest/SpatialComponentTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialComponentTest/SpatialComponentTestReplicatedActor.h"

USpatialComponentMap::USpatialComponentMap()
{
	MapCategory = CI_FAST;
	MapName = TEXT("SpatialComponentMap");
}

void USpatialComponentMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the test
	ASpatialComponentTest* CompTest = CastChecked<ASpatialComponentTest>(
		GEditor->AddActor(CurrentLevel, ASpatialComponentTest::StaticClass(), FTransform(FVector(-250, -250, 0))));

	// Add the helpers, as we need things placed in the level
	CompTest->LevelActor = CastChecked<ASpatialComponentTestActor>(
		GEditor->AddActor(CurrentLevel, ASpatialComponentTestActor::StaticClass(), FTransform(FVector(-250, -250, 0))));
	CompTest->LevelReplicatedActor = CastChecked<ASpatialComponentTestReplicatedActor>(
		GEditor->AddActor(CurrentLevel, ASpatialComponentTestReplicatedActor::StaticClass(), FTransform(FVector(-250, -250, 0))));

	// Quirk of the test. We need the player spawns on the same portion of the map as the test, so they are LBed together
	AActor** PlayerStart = CurrentLevel->Actors.FindByPredicate([](AActor* Actor) {
		return Actor->GetClass() == APlayerStart::StaticClass();
	});
	(*PlayerStart)->SetActorLocation(FVector(-500, -250, 100));

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest1x2WorkerSettings::StaticClass());
	WorldSettings->DefaultGameMode = ASpatialAuthorityTestGameMode::StaticClass();
}
