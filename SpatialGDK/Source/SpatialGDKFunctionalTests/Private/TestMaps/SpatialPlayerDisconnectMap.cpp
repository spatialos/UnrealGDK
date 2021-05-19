// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialPlayerDisconnectMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/PlayerDisconnect/SpatialTestPlayerDisconnect.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/PlayerDisconnect/SpatialPlayerDisconnectGameMode.h"
#include "TestWorkerSettings.h"

USpatialPlayerDisconnectMap::USpatialPlayerDisconnectMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE, TEXT("SpatialPlayerDisconnectMap"))
{
}

void USpatialPlayerDisconnectMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Quirk of the test. We need the player spawn close to the ground so that they do not generate an intermittent error when falling
	AActor** PlayerStart = CurrentLevel->Actors.FindByPredicate([](AActor* Actor) {
		return Actor->GetClass() == APlayerStart::StaticClass();
	});
	(*PlayerStart)->SetActorLocation(FVector(-190, 0, 50));

	// Add the test
	ASpatialTestPlayerDisconnect& TriggerTest =
		AddActorToLevel<ASpatialTestPlayerDisconnect>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = ASpatialPlayerDisconnectGameMode::StaticClass();
}
