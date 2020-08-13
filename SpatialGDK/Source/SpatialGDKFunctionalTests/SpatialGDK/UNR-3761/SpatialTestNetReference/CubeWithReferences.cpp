// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CubeWithReferences.h"
#include "Net/UnrealNetwork.h"

ACubeWithReferences::ACubeWithReferences()
{
	bNetLoadOnClient = false;
	bNetLoadOnNonAuthServer = true;
}

int ACubeWithReferences::CountValidNeighbours()
{
	int ValidNeighbours = 0;

	if (IsValid(Neighbour1))
	{
		ValidNeighbours++;
	}

	if (IsValid(Neighbour2))
	{
		ValidNeighbours++;
	}

	return ValidNeighbours;
}

void ACubeWithReferences::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACubeWithReferences, Neighbour1);
	DOREPLIFETIME(ACubeWithReferences, Neighbour2);
}
