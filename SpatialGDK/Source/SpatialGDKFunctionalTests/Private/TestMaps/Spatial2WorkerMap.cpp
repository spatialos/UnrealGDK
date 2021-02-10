// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/Spatial2WorkerMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/Public/Test1x2WorkerSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/CrossServerAndClientOrchestrationTest/CrossServerAndClientOrchestrationTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/RegisterAutoDestroyActorsTest/RegisterAutoDestroyActorsTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/RelevancyTest/RelevancyTest.h"

USpatial2WorkerMap::USpatial2WorkerMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("Spatial2WorkerMap"))
{
}

void USpatial2WorkerMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// The two orchestration tests are placed at opposite points around the origin to "guarantee" they will land on different workers so
	// that they can demonstrate they work in all situations
	AddActorToLevel<ACrossServerAndClientOrchestrationTest>(CurrentLevel, FTransform(FVector(-250, 250, 0)));
	AddActorToLevel<ACrossServerAndClientOrchestrationTest>(CurrentLevel, FTransform(FVector(250, -250, 0)));
	AddActorToLevel<ARegisterAutoDestroyActorsTestPart1>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ARegisterAutoDestroyActorsTestPart2>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ARelevancyTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest1x2WorkerSettings::StaticClass());
}
