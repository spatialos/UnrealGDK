// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/Spatial2WorkerSmallInterestMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/Public/SpatialCleanupConnectionTest.h" //didn't spot this during review, but I would argue this one is "SpatialGDK" as well
#include "SpatialGDKFunctionalTests/Public/SpatialTest1x2GridSmallInterestWorkerSettings.h"

USpatial2WorkerSmallInterestMap::USpatial2WorkerSmallInterestMap()
{
	MapCategory = CI_FAST_SPATIAL_ONLY;
	MapName = TEXT("Spatial2WorkerSmallInterestMap");
}

void USpatial2WorkerSmallInterestMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	GEditor->AddActor(CurrentLevel, ASpatialCleanupConnectionTest::StaticClass(),
					  FTransform(FVector(-50, -50, 0))); // Seems like this position is required so that the LB plays nicely?

	// Quirk of the test. We need the player spawns on the same portion of the map as the test, so they are LBed together
	AActor** PlayerStart = CurrentLevel->Actors.FindByPredicate([](AActor* Actor) {
		return Actor->GetClass() == APlayerStart::StaticClass();
	});
	(*PlayerStart)->SetActorLocation(FVector(-50, -50, 100));

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(USpatialTest1x2GridSmallInterestWorkerSettings::StaticClass());
}
