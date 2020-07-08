// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "TestMovementCharacter.h"
#include "Engine/Classes/Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Components/CapsuleComponent.h"

ATestMovementCharacter::ATestMovementCharacter()
{
	bReplicates = true;
	bReplicateMovement = true;

	GetCapsuleComponent()->InitCapsuleSize(38.0f, 38.0f);

	SphereComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereComponent"));
	SphereComponent->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'")));
	SphereComponent->SetMaterial(0, LoadObject<UMaterial>(nullptr, TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'")));
	SphereComponent->SetVisibility(true);
	SphereComponent->SetupAttachment(GetCapsuleComponent());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->bAbsoluteLocation = false;
	CameraComponent->bAbsoluteRotation = false;
	CameraComponent->RelativeLocation = FVector(300.0f, 0.0f, 75.0f);
	CameraComponent->RelativeRotation = FRotator::MakeFromEuler(FVector(0.0f, -10.0f, 180.0f));
	CameraComponent->SetupAttachment(GetCapsuleComponent());
}

void ATestMovementCharacter::ServerMoveToLocation_Implementation(FVector Destination)
{
	SetActorLocation(Destination);
}
