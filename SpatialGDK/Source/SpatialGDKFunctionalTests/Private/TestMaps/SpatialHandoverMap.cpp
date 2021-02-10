// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialHandoverMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/Public/Test2x2WorkerSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3761/SpatialTestHandover/SpatialTestHandover.h"

USpatialHandoverMap::USpatialHandoverMap()
	// This test and map is in CI_NIGHTLY_SPATIAL_ONLY, because I cannot run it a 100 times in a row
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("SpatialHandoverMap"))
{
}

void USpatialHandoverMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	AddActorToLevel<ASpatialTestHandover>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest2x2WorkerSettings::StaticClass());
}
