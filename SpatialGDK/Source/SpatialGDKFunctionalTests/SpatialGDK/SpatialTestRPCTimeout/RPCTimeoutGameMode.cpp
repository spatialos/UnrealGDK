// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "RPCTimeoutGameMode.h"

#include "RPCTimeoutCharacter.h"
#include "RPCTimeoutPlayerController.h"

ARPCTimeoutGameMode::ARPCTimeoutGameMode()
	: Super()
{
	PlayerControllerClass = ARPCTimeoutPlayerController::StaticClass();
	DefaultPawnClass = ARPCTimeoutCharacter::StaticClass();
}
