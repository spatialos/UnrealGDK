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

void ATestPossessionPlayerController::OnPossessFailed(ERemotePossessFailure FailureReason)
{
	Super::OnPossessFailed(FailureReason);
	OnPossessFailureEvent.Broadcast(FailureReason, this);
}
