// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CrossServerPossessionGameMode.generated.h"

UCLASS()
class ACrossServerPossessionGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ACrossServerPossessionGameMode();
	AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;

private:
	AActor* SpawnPoint;
};
