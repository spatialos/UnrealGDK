// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialPropertyReplicationMap.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPropertyReplication/SpatialTestPropertyReplication.h"

USpatialPropertyReplicationMap::USpatialPropertyReplicationMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE, TEXT("SpatialPropertyReplicationMap"))
{
	// clang-format off
	SetCustomConfig(TEXT("[/Script/UnrealEd.LevelEditorPlaySettings]") LINE_TERMINATOR
					TEXT("PlayNumberOfClients=3"));
	// clang-format on
}

void USpatialPropertyReplicationMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	AddActorToLevel<ASpatialTestPropertyReplication>(CurrentLevel, FTransform::Identity);
}
