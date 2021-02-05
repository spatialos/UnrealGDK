// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialHandoverMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/Public/Test2x2WorkerSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3761/SpatialTestHandover/SpatialTestHandover.h"

USpatialHandoverMap::USpatialHandoverMap()
{
	MapCategory = CI_SLOW_SPATIAL_ONLY; // This test and map is here, because I cannot run it a 100 times in a row
	MapName = TEXT("SpatialHandoverMap");
}

void USpatialHandoverMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	GEditor->AddActor(CurrentLevel, ASpatialTestHandover::StaticClass(), FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest2x2WorkerSettings::StaticClass());
	WorldSettings->bEnableDebugInterface =
		false; // The test directly accesses the LayeredStrategy and expects it to be the top one, setting to false for now
}
