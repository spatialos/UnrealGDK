// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SimulatedPlayers/SimPlayerBPFunctionLibrary.h"

#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogSimulatedPlayer, Log, All);

bool USimPlayerBPFunctionLibrary::IsSimulatedPlayer(const UObject* WorldContextObject)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (GameInstance == nullptr)
	{
		UE_LOG(LogSimulatedPlayer, Warning, TEXT("Cannot check if player is simulated: cannot get GameInstance. Defaulting to false."));
		return false;
	}

	return GameInstance->IsSimulatedPlayer();
}
