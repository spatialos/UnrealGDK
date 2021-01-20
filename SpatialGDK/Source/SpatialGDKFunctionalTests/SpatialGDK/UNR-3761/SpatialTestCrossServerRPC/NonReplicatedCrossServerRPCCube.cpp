// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "NonReplicatedCrossServerRPCCube.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Net/UnrealNetwork.h"

ANonReplicatedCrossServerRPCCube::ANonReplicatedCrossServerRPCCube()
{
	bAlwaysRelevant = true;
	bNetLoadOnClient = true;
	bNetLoadOnNonAuthServer = true;

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
	SetRole(ROLE_SimulatedProxy);
	SetRemoteRoleForBackwardsCompat(ROLE_Authority);
}
