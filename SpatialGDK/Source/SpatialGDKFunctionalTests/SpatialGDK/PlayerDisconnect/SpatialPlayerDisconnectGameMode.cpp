// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPlayerDisconnectGameMode.h"
#include "PlayerDisconnectController.h"
#include "EngineClasses/SpatialGameInstance.h"


ASpatialPlayerDisconnectGameMode::ASpatialPlayerDisconnectGameMode()
	: Super()
{
	PlayerControllerClass = APlayerDisconnectController::StaticClass();

	static ConstructorHelpers::FClassFinder<AActor> PawnClassFinder(TEXT("/Game/Characters/PlayerCharacter_BP"));
	DefaultPawnClass = PawnClassFinder.Class;
}


