// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/Spatial2WorkerMigrationMaps.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMigration/SpatialTestCharacterMigration.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMovement/CharacterMovementTestGameMode.h"
#include "TestWorkerSettings.h"

USpatial2WorkerFullInterestMigrationMap::USpatial2WorkerFullInterestMigrationMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("Spatial2WorkerFullInterestMigrationMap"))
{
}

void USpatial2WorkerFullInterestMigrationMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	AddActorToLevel<ASpatialTestCharacterMigration>(CurrentLevel, FTransform::Identity);

	FTransform PlayerTransform;

	PlayerTransform.SetTranslation(FVector(-500.0f, -500.0f, 50.0f));
	AddActorToLevel<APlayerStart>(CurrentLevel, PlayerTransform);

	PlayerTransform.SetTranslation(FVector(-500.0f, 500.0f, 50.0f));
	AddActorToLevel<APlayerStart>(CurrentLevel, PlayerTransform);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = ACharacterMovementTestGameMode::StaticClass();
	WorldSettings->SetMultiWorkerSettingsClass(UTest2x1FullInterestWorkerSettings::StaticClass());
}

USpatial2WorkerNoInterestMigrationMap::USpatial2WorkerNoInterestMigrationMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("Spatial2WorkerNoInterestMigrationMap"))
{
}

void USpatial2WorkerNoInterestMigrationMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	AddActorToLevel<ASpatialTestCharacterMigration>(CurrentLevel, FTransform::Identity);

	FTransform PlayerTransform;

	PlayerTransform.SetTranslation(FVector(-500.0f, -500.0f, 50.0f));
	AddActorToLevel<APlayerStart>(CurrentLevel, PlayerTransform);

	PlayerTransform.SetTranslation(FVector(-500.0f, 500.0f, 50.0f));
	AddActorToLevel<APlayerStart>(CurrentLevel, PlayerTransform);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = ACharacterMovementTestGameMode::StaticClass();
	WorldSettings->SetMultiWorkerSettingsClass(UTest2x1NoInterestWorkerSettings::StaticClass());
}

