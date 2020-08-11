// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedTestActorBase.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"

AReplicatedTestActorBase::AReplicatedTestActorBase()
{
	bReplicates = true;

	CubeComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeComponent"));
	CubeComponent->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")));
	CubeComponent->SetMaterial(0,
							   LoadObject<UMaterial>(nullptr, TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'")));
	CubeComponent->SetVisibility(true);

	RootComponent = CubeComponent;
}
