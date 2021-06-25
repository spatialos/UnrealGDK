// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/Spatial2WorkerMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/CrossServerAndClientOrchestrationTest/CrossServerAndClientOrchestrationTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/RegisterAutoDestroyActorsTest/RegisterAutoDestroyActorsTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/RelevancyTest/RelevancyTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestMultiServerUnrealComponents/SpatialTestMultiServerUnrealComponents.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPropertyReplication/SpatialTestPropertyReplicationMultiworker.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestReplicationConditions/SpatialTestReplicationConditions.h"
#include "TestWorkerSettings.h"

USpatial2WorkerMap::USpatial2WorkerMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("Spatial2WorkerMap"))
{
	SetNumberOfClients(2);
}

void USpatial2WorkerMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	FTransform Server1Pos(FVector(250, -250, 0));
	FTransform Server2Pos(FVector(-250, 250, 0));

	// The two orchestration tests are placed at opposite points around the origin to "guarantee" they will land on different workers so
	// that they can demonstrate they work in all situations
	AddActorToLevel<ACrossServerAndClientOrchestrationTest>(CurrentLevel, Server2Pos);
	AddActorToLevel<ACrossServerAndClientOrchestrationTest>(CurrentLevel, Server1Pos);
	AddActorToLevel<ARegisterAutoDestroyActorsTestPart1>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ARegisterAutoDestroyActorsTestPart2>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ARelevancyTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ASpatialTestPropertyReplicationMultiworker>(CurrentLevel, Server1Pos);
	// Test actor is placed in Server 1 load balancing area to ensure Server 1 becomes authoritative.
	AddActorToLevel<ASpatialTestMultiServerUnrealComponents>(CurrentLevel, Server1Pos);
	AddActorToLevel<ASpatialTestReplicationConditions>(CurrentLevel, Server1Pos);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest1x2FullInterestWorkerSettings::StaticClass());
}
