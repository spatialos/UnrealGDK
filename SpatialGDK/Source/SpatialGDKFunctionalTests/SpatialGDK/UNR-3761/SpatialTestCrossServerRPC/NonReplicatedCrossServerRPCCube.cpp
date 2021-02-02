// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "NonReplicatedCrossServerRPCCube.h"

ANonReplicatedCrossServerRPCCube::ANonReplicatedCrossServerRPCCube()
{
	bReplicates = false;
	SetReplicateMovement(false);
}

void ANonReplicatedCrossServerRPCCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ANonReplicatedCrossServerRPCCube::TurnOnReplication()
{
	SetReplicates(true);
	SetReplicateMovement(true);
}

void ANonReplicatedCrossServerRPCCube::SetNonAuth()
{
	Role = ROLE_SimulatedProxy;
	RemoteRole = ROLE_Authority;
}
