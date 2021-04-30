// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/DefaultPawn.h"
#include "CrossServerPossessionGameMode.generated.h"

UCLASS()
class ACrossServerPossessionTestPawn : public ADefaultPawn
{
	GENERATED_BODY()
	public:
	ACrossServerPossessionTestPawn();
};

UCLASS()
class ACrossServerPossessionGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ACrossServerPossessionGameMode();
	AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

private:
	void Generate_SpawnPoints();

	int32 PlayersSpawned;
	bool bInitializedSpawnPoints;
	TArray<AActor*> SpawnPoints;
	TMap<int32, AActor*> PlayerIdToSpawnPointMap;
};
