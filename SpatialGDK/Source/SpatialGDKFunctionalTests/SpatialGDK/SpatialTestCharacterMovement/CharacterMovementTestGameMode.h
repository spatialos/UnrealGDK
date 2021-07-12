// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CharacterMovementTestGameMode.generated.h"

/**
 * GameMode used for the SpatialTestCharacterMovementMap
 */
UCLASS()
class ACharacterMovementTestGameMode : public AGameModeBase
{
	GENERATED_UCLASS_BODY()

	virtual void BeginPlay() override;
	AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

private:

	int32 SpawnRequests = 0;
	TArray<APlayerStart*> PlayerStarts;
};
