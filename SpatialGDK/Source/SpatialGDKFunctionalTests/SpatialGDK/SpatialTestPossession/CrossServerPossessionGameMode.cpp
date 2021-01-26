// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "TestPossessionPlayerController.h"

ACrossServerPossessionGameMode::ACrossServerPossessionGameMode()
	: SpawnPoint(nullptr)
{
	PlayerControllerClass = ATestPossessionPlayerController::StaticClass();
}

AActor* ACrossServerPossessionGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	if (nullptr == SpawnPoint)
	{
		UWorld* World = GetWorld();
		FActorSpawnParameters SpawnInfo{};
		SpawnInfo.Owner = this;
		SpawnInfo.Instigator = NULL;
		SpawnInfo.bDeferConstruction = false;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnPoint = World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FVector(-500.0f, -500.0f, 50.0f), FRotator::ZeroRotator,
													 SpawnInfo);
	}
	return SpawnPoint;
}
