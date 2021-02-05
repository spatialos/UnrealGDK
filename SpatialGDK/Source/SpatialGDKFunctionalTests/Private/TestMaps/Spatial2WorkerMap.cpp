// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/Spatial2WorkerMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/Public/Test1x2WorkerSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/CrossServerAndClientOrchestrationTest/CrossServerAndClientOrchestrationTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/RegisterAutoDestroyActorsTest/RegisterAutoDestroyActorsTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/RelevancyTest/RelevancyTest.h"

USpatial2WorkerMap::USpatial2WorkerMap()
{
	MapCategory = CI_FAST_SPATIAL_ONLY;
	MapName = TEXT("Spatial2WorkerMap");
}

void USpatial2WorkerMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// The two orchestration tests are placed at opposite points around the origin to "guarantee" they will land on different workers so
	// that they can demonstrate they work in all situations
	GEditor->AddActor(CurrentLevel, ACrossServerAndClientOrchestrationTest::StaticClass(), FTransform(FVector(-250, 250, 0)));
	GEditor->AddActor(CurrentLevel, ACrossServerAndClientOrchestrationTest::StaticClass(), FTransform(FVector(250, -250, 0)));
	// This one lives in the TestGyms, TODO
	// GEditor->AddActor(CurrentLevel, APredictedGameplayCuesTest::StaticClass(), FTransform::Identity);
	// This one lives in the TestGyms, TODO
	// GEditor->AddActor(CurrentLevel, ACrossServerAbilityActivationTest::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARegisterAutoDestroyActorsTestPart1::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARegisterAutoDestroyActorsTestPart2::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARelevancyTest::StaticClass(), FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest1x2WorkerSettings::StaticClass());
}
