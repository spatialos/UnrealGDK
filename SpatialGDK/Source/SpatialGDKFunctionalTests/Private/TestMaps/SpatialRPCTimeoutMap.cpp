// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/SpatialRPCTimeoutMap.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestRPCTimeout/SpatialTestRPCTimeout.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestRPCTimeout/SpatialTestRPCTimeoutGameMode.h"

USpatialRPCTimeoutMap::USpatialRPCTimeoutMap()
	: UGeneratedTestMap(EMapCategory::CI_PREMERGE, TEXT("SpatialRPCTimeoutMap"))
{
	// clang-format off
	SetCustomConfig(TEXT("[/Script/SpatialGDK.SpatialGDKSettings]") LINE_TERMINATOR
		TEXT("QueuedIncomingRPCWaitTime=0") LINE_TERMINATOR
		TEXT("[/Script/UnrealEd.LevelEditorPlaySettings]") LINE_TERMINATOR
		TEXT("PlayNumberOfClients=2")); // LINE_TERMINATOR
		//TEXT("RunUnderOneProcess=true"));
	// clang-format on
}

void USpatialRPCTimeoutMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();
	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = ASpatialTestRPCTimeoutGameMode::StaticClass();

	// Add the test
	FTransform Transform1 = FTransform::Identity;
	Transform1.SetLocation(FVector(-300.f, -500.f, 200.f));

	AddActorToLevel<APlayerStart>(CurrentLevel, Transform1);
	AddActorToLevel<ASpatialTestRPCTimeout>(CurrentLevel, FTransform::Identity);
}
