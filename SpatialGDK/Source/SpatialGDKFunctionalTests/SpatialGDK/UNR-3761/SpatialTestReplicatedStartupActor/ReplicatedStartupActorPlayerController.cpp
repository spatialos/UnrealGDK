// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedStartupActorPlayerController.h"
#include "SpatialTestReplicatedStartupActor.h"

void AReplicatedStartupActorPlayerController::ClientToServerRPC_Implementation(ASpatialTestReplicatedStartupActor* Test,
																			   AActor* ReplicatedActor)
{
	if (IsValid(ReplicatedActor))
	{
		Test->bIsValidReference = true;
	}
}

void AReplicatedStartupActorPlayerController::ResetBoolean_Implementation(ASpatialTestReplicatedStartupActor* Test)
{
	Test->bIsValidReference = false;
}
