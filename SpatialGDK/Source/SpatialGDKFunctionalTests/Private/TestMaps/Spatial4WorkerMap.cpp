// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/Spatial4WorkerMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/Public/Test2x2WorkerSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestHandoverReplication/SpatialTestHandoverDynamicReplication.h"

USpatial4WorkerMap::USpatial4WorkerMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("Spatial4WorkerMap"))
{
}

void USpatial4WorkerMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	AddActorToLevel<ASpatialTestHandoverDynamicReplication>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest2x2WorkerSettings::StaticClass());
}
