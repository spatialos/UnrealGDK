// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "NetOwnershipCube.h"
#include "Net/UnrealNetwork.h"

ANetOwnershipCube::ANetOwnershipCube()
{
	SetReplicateMovement(true);

	ReceivedRPCs = 0;
}

void ANetOwnershipCube::ServerIncreaseRPCCount_Implementation()
{
	ReceivedRPCs++;
}

void ANetOwnershipCube::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetOwnershipCube, ReceivedRPCs);
}
