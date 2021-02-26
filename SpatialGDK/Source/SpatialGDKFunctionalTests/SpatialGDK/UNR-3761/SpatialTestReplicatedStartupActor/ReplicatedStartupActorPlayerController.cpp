// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedStartupActorPlayerController.h"
#include "SpatialReplicatedStartupActorTest.h"

void AReplicatedStartupActorPlayerController::ClientToServerRPC_Implementation(ASpatialReplicatedStartupActorTest* Test,
																			   AActor* ReplicatedActor)
{
	if (IsValid(ReplicatedActor))
	{
		Test->bIsValidReference = true;
	}
}

void AReplicatedStartupActorPlayerController::ResetBoolean_Implementation(ASpatialReplicatedStartupActorTest* Test)
{
	Test->bIsValidReference = false;
}
