// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestPossessionPlayerController.h"
#include "Engine/World.h"

ATestPossessionPlayerController::ATestPossessionPlayerController() {}

void ATestPossessionPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	OnPossessEvent.Broadcast(InPawn, this);
}

void ATestPossessionPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	OnUnPossessEvent.Broadcast(this);
}
