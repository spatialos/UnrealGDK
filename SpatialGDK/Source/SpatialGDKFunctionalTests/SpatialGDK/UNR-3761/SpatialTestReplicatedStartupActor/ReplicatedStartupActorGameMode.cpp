// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedStartupActorGameMode.h"
#include "ReplicatedStartupActorPlayerController.h"

AReplicatedStartupActorGameMode::AReplicatedStartupActorGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = AReplicatedStartupActorPlayerController::StaticClass();
}
