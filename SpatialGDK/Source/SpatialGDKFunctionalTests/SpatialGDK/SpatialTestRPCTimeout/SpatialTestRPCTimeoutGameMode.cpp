// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialTestRPCTimeoutGameMode.h"

#include "SpatialTestRPCTimeoutCharacter.h"
#include "SpatialTestRPCTimeoutPlayerController.h"

ARPCTimeoutGameMode::ARPCTimeoutGameMode()
	: Super()
{
	PlayerControllerClass = ARPCTimeoutPlayerController::StaticClass();
	DefaultPawnClass = ARPCTimeoutCharacter::StaticClass();
}
