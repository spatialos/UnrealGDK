// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestPossessionPlayerController.h"
#include "Engine/World.h"
#include "EngineClasses/SpatialPossession.h"

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
	OnPossessFailedEvent.Broadcast(FailureReason, this);
}

void ATestPossessionPlayerController::RemotePossess_Implementation(APawn* InPawn)
{
	USpatialPossession::RemotePossess(this, InPawn);
}
