// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerRPCCube.h"
#include "Net/UnrealNetwork.h"

ACrossServerRPCCube::ACrossServerRPCCube()
{
	bAlwaysRelevant = true;
	bNetLoadOnClient = true;
	bNetLoadOnNonAuthServer = true;
}

void ACrossServerRPCCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACrossServerRPCCube, ReceivedCrossServerRPCS);
}

void ACrossServerRPCCube::CrossServerTestRPC_Implementation(int SendingServerID)
{
	ReceivedCrossServerRPCS.Add(SendingServerID);
}
