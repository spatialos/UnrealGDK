// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SimulatedPlayer.h"

#include "Engine.h"
#include "Misc/Parse.h"
#include "Misc/CommandLine.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY_STATIC(LogSimulatedPlayer, Log, All);

bool USimulatedPlayer::IsSimulatedPlayer(UObject* WorldContextObject)
{
	UEngine* engine = GEngine;
	if (engine == nullptr)
	{
		UE_LOG(LogSimulatedPlayer, Error, TEXT("Cannot check if player is simulated: GEngine is null. Defaulting to false."));
		return false;
	}

	UWorld* world = engine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (world == nullptr)
	{
		UE_LOG(LogSimulatedPlayer, Error, TEXT("Cannot check if player is simulated: world returned by WorldContextObject is null. Defaulting to false."));
		return false;
	}

	UGameInstance* gameInstance = world->GetGameInstance();
	if (gameInstance == nullptr)
	{
		UE_LOG(LogSimulatedPlayer, Error, TEXT("Cannot check if player is simulated: game instance is null. Defaulting to false."));
		return false;
	}

	return gameInstance->IsSimulatedPlayer();
}
