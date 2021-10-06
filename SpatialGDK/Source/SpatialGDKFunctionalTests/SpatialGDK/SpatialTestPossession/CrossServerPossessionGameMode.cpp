// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestPawnBase_RepGraphAlwaysReplicate.h"
#include "TestPossessionPlayerController.h"

ACrossServerPossessionGameMode::ACrossServerPossessionGameMode()
	: PlayersSpawned(0)
	, bInitializedSpawnPoints(false)
{
	DefaultPawnClass = ATestPawnBase_RepGraphAlwaysReplicate::StaticClass();
	PlayerControllerClass = ATestPossessionPlayerController::StaticClass();
}

AActor* ACrossServerPossessionGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	Generate_SpawnPoints();

	if (Player == nullptr)
	{
		return SpawnPoints[PlayersSpawned % SpawnPoints.Num()];
	}

	const int32 PlayerUniqueID = Player->GetUniqueID();
	AActor** SpawnPoint = PlayerIdToSpawnPointMap.Find(PlayerUniqueID);
	if (SpawnPoint != nullptr)
	{
		return *SpawnPoint;
	}

	AActor* ChosenSpawnPoint = SpawnPoints[PlayersSpawned % SpawnPoints.Num()];
	PlayerIdToSpawnPointMap.Add(PlayerUniqueID, ChosenSpawnPoint);

	PlayersSpawned++;

	return ChosenSpawnPoint;
}

void ACrossServerPossessionGameMode::Generate_SpawnPoints()
{
	if (false == bInitializedSpawnPoints)
	{
		UWorld* World = GetWorld();

		FActorSpawnParameters SpawnInfo{};
		SpawnInfo.Owner = this;
		SpawnInfo.Instigator = NULL;
		SpawnInfo.bDeferConstruction = false;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SpawnPoints.Add(World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FVector(-500.0f, -500.0f, 50.0f),
														FRotator::ZeroRotator, SpawnInfo));
		SpawnPoints.Add(World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FVector(500.0f, -500.0f, 50.0f), FRotator::ZeroRotator,
														SpawnInfo));
		SpawnPoints.Add(World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), FVector(-500.0f, 500.0f, 50.0f), FRotator::ZeroRotator,
														SpawnInfo));

		bInitializedSpawnPoints = true;
	}
}
