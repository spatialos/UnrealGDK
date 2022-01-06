// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRPCTimeoutGameMode.h"

#include "SpatialTestRPCTimeoutCharacter.h"
#include "SpatialTestRPCTimeoutPlayerController.h"

ASpatialTestRPCTimeoutGameMode::ASpatialTestRPCTimeoutGameMode()
	: Super()
{
	PlayerControllerClass = ASpatialTestRPCTimeoutPlayerController::StaticClass();
	DefaultPawnClass = ASpatialTestRPCTimeoutCharacter::StaticClass();
}
