// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PartiallyStablePathPawn.h"

#include "PartiallyStablePathActor.h"

void APartiallyStablePathPawn::ServerVerifyComponentReference_Implementation(APartiallyStablePathActor* Actor,
																			 UStaticMeshComponent* Component)
{
	bServerRPCCalled = true;
	bRPCParamMatchesComponent = Actor != nullptr && Component != nullptr && Actor->Component == Component;
}

bool APartiallyStablePathPawn::ServerVerifyComponentReference_Validate(APartiallyStablePathActor* Actor, UStaticMeshComponent* Component)
{
	return true;
}
