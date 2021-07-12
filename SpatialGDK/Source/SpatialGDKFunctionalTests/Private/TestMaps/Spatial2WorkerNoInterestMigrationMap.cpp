// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/Spatial2WorkerNoInterestMigrationMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMigration/SpatialTestCharacterMigration.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMovement/CharacterMovementTestGameMode.h"
#include "TestWorkerSettings.h"

USpatial2WorkerNoInterestMigrationMap::USpatial2WorkerNoInterestMigrationMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("Spatial2WorkerNoInterestMigrationMap"))
{
	SetNumberOfClients(2);
}

void USpatial2WorkerNoInterestMigrationMap::CreateCustomContentForMap()
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
	WorldSettings->SetMultiWorkerSettingsClass(UTest2x1NoInterestWorkerSettings::StaticClass());
	WorldSettings->DefaultGameMode = ACharacterMovementTestGameMode::StaticClass();
}
