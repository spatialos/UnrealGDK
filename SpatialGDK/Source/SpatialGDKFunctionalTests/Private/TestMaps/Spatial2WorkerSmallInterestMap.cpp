// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/Spatial2WorkerSmallInterestMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/AlwaysInterestedTest/AlwaysInterestedTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialCleanupConnectionTest/SpatialCleanupConnectionTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestHandoverReplication/SpatialTestHandoverActorComponentReplication.h"
#include "TestWorkerSettings.h"

USpatial2WorkerSmallInterestMap::USpatial2WorkerSmallInterestMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("Spatial2WorkerSmallInterestMap"))
{
}

void USpatial2WorkerSmallInterestMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	FVector Server1Pos(-50.f, -50.f, 0.f);

	// Add the tests
	AddActorToLevel<ASpatialCleanupConnectionTest>(
		CurrentLevel, FTransform(Server1Pos)); // Seems like this position is required so that the LB plays nicely?
	AddActorToLevel<ASpatialTestHandoverActorComponentReplication>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<AAlwaysInterestedTest>(CurrentLevel, FTransform(Server1Pos));

	// Quirk of the test. We need the player spawns on the same portion of the map as the test, so they are LBed together
	AActor** PlayerStart = CurrentLevel->Actors.FindByPredicate([](AActor* Actor) {
		return Actor->GetClass() == APlayerStart::StaticClass();
	});
	(*PlayerStart)->SetActorLocation(FVector(-50, -50, 100));

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest1x2SmallInterestWorkerSettings::StaticClass());
}
