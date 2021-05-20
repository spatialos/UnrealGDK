// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPlayerDisconnectGameMode.h"
#include "EngineClasses/SpatialGameInstance.h"
#include "GameFramework/Character.h"
#include "PlayerDisconnectController.h"

ASpatialPlayerDisconnectGameMode::ASpatialPlayerDisconnectGameMode()
	: Super()
{
	PlayerControllerClass = APlayerDisconnectController::StaticClass();
	DefaultPawnClass = ACharacter::StaticClass();
}
