// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialRoutingWorkerMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestEntityInteration/EntityInteractionTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestEntityInteration/EntityInteractionTestActor.h"
#include "TestWorkerSettings.h"

USpatialRoutingWorkerMap::USpatialRoutingWorkerMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("SpatialRoutingWorkerMap"))
{
	SetNumberOfClients(2);
	// clang-format off
	SetCustomConfig(TEXT("[/Script/SpatialGDK.SpatialGDKSettings]") LINE_TERMINATOR
					TEXT("CrossServerRPCImplementation=RoutingWorker"));
	// clang-format on
}

void USpatialRoutingWorkerMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	FTransform Server1Pos(FVector(250, -250, 0));
	FTransform Server2Pos(FVector(-250, 250, 0));

	AddActorToLevel<AEntityInteractionTestActor>(CurrentLevel, Server2Pos).Index = 0;
	AddActorToLevel<AEntityInteractionTestActor>(CurrentLevel, Server2Pos).Index = 1;
	AddActorToLevel<AEntityInteractionTestActor>(CurrentLevel, Server1Pos).Index = 0;
	AddActorToLevel<AEntityInteractionTestActor>(CurrentLevel, Server1Pos).Index = 1;
	AddActorToLevel<ASpatialEntityInteractionTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest1x2FullInterestWorkerSettings::StaticClass());
}
