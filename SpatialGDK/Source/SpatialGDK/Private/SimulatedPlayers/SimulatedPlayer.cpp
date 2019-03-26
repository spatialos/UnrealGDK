// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SimulatedPlayer.h"

#include "Misc/Parse.h"
#include "Misc/CommandLine.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY_STATIC(LogSimulatedPlayer, Log, All);

bool USimulatedPlayer::IsSimulatedPlayer()
{
#if WITH_EDITOR
	// Check game instance instead of command line arguments for PIE simulated players.
	UWorld* world = GWorld;
	if (world == nullptr)
	{
		UE_LOG(LogSimulatedPlayer, Warning, TEXT("Cannot check if player is simulated: GWorld is null. Defaulting to false."));
		return false;
	}

	UGameInstance* gameInstance = world->GetGameInstance();
	if (gameInstance == nullptr)
	{
		UE_LOG(LogSimulatedPlayer, Warning, TEXT("Cannot check if player is simulated: game instance is null. Defaulting to false."));
		return false;
	}

	return gameInstance->IsSimulatedPlayer();
#endif // WITH_EDITOR

	return FParse::Param(FCommandLine::Get(), *SpatialConstants::SimulatedPlayerArg);
}
