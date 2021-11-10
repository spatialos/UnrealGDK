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
		TEXT("QueuedIncomingRPCWaitTime=0"));
	// clang-format on

	SetNumberOfClients(2);

	EnableMultiProcess();
}

void USpatialRPCTimeoutMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();
	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = ASpatialTestRPCTimeoutGameMode::StaticClass();

	FTransform Transform1 = FTransform::Identity;
	Transform1.SetLocation(FVector(-300.f, -500.f, 200.f));

	// Added a second player start actor to ensure both characters are visible from the game window for debugging
	AddActorToLevel<APlayerStart>(CurrentLevel, Transform1);
	AddActorToLevel<ASpatialTestRPCTimeout>(CurrentLevel, FTransform::Identity);
}
