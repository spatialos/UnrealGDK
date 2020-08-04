// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "MultipleOwnershipPawn.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Materials/Material.h"
#include "Net/UnrealNetwork.h"

AMultipleOwnershipPawn::AMultipleOwnershipPawn()
{
	bReplicates = true;
#if ENGINE_MINOR_VERSION < 24
	bReplicateMovement = true;
#else
	SetReplicatingMovement(true);
#endif
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);

	RootComponent = CollisionComponent;

	CubeComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeComponent"));
	CubeComponent->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")));
	CubeComponent->SetMaterial(0, LoadObject<UMaterial>(nullptr, TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'")));
	CubeComponent->SetVisibility(true);
	CubeComponent->SetupAttachment(RootComponent);

	ReceivedRPCs = 0;
}

void AMultipleOwnershipPawn::ServerSendRPC_Implementation()
{
	++ReceivedRPCs;
}

void AMultipleOwnershipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultipleOwnershipPawn, ReceivedRPCs);
}
