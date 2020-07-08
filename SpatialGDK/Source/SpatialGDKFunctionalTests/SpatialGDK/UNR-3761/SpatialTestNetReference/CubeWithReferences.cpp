// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "CubeWithReferences.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Net/UnrealNetwork.h"

ACubeWithReferences::ACubeWithReferences()
{
	bReplicates = true;
	bNetLoadOnClient = false;
	bNetLoadOnNonAuthServer = true;

	CubeComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeComponent"));
	CubeComponent->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")));
	CubeComponent->SetMaterial(0, LoadObject<UMaterial>(nullptr, TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'")));
	CubeComponent->SetVisibility(true);

	RootComponent = CubeComponent;
}

int ACubeWithReferences::CountValidNeighbours()
{
	int ValidNeighbours = 0;

	if (IsValid(Neighbour1))
	{
	 	ValidNeighbours ++;
	}

	if (IsValid(Neighbour2))
	{
		ValidNeighbours ++;
	}

	return ValidNeighbours;
}

void ACubeWithReferences::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACubeWithReferences, Neighbour1);
	DOREPLIFETIME(ACubeWithReferences, Neighbour2);
}
