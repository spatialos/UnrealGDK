// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/Spatial2WorkerFullInterestMigrationMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMigration/SpatialTestCharacterMigration.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMovement/CharacterMovementTestGameMode.h"
#include "TestWorkerSettings.h"

USpatial2WorkerFullInterestMigrationMap::USpatial2WorkerFullInterestMigrationMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("Spatial2WorkerFullInterestMigrationMap"))
{
	SetNumberOfClients(2);
}

void USpatial2WorkerFullInterestMigrationMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	AddActorToLevel<ASpatialTestCharacterMigration>(CurrentLevel, FTransform::Identity);

	FTransform PlayerTransform;

	PlayerTransform.SetTranslation(FVector(-500, -500, 200));
	AddActorToLevel<APlayerStart>(CurrentLevel, PlayerTransform);

	PlayerTransform.SetTranslation(FVector(-500, 500, 200));
	AddActorToLevel<APlayerStart>(CurrentLevel, PlayerTransform);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest2x1FullInterestWorkerSettings::StaticClass());
	WorldSettings->DefaultGameMode = ACharacterMovementTestGameMode::StaticClass();
}
