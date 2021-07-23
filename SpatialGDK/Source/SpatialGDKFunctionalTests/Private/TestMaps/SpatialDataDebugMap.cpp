// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialDataDebugMap.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPropertyReplication/SpatialTestPropertyReplicationMultiworker.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPropertyReplication/SpatialTestPropertyReplicationSubobject.h"
#include "TestWorkerSettings.h"

USpatialDataDebugMap::USpatialDataDebugMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE_SPATIAL_ONLY, TEXT("SpatialDataDebugMap"))
{
	SetNumberOfClients(2);

	// clang-format off
	SetCustomConfig(TEXT("[/Script/SpatialGDK.SpatialGDKSettings]") LINE_TERMINATOR
					TEXT("bSpatialAuthorityDebugger=True"));
	// clang-format on
}

void USpatialDataDebugMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	FTransform Server1Pos(FVector(250, -250, 0));
	FTransform Server2Pos(FVector(-250, 250, 0)); // TODO: keep this for now in case we need it for other tests

	AddActorToLevel<ASpatialTestPropertyReplicationMultiworker>(CurrentLevel, Server1Pos);
	AddActorToLevel<ASpatialTestPropertyReplicationSubobject>(CurrentLevel, Server1Pos);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest1x2FullInterestWorkerSettings::StaticClass());
}
