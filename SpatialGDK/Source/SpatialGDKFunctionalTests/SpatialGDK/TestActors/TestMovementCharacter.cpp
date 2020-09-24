// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMovementCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Classes/Camera/CameraComponent.h"
#include "Materials/Material.h"
#include "Net/UnrealNetwork.h"

ATestMovementCharacter::ATestMovementCharacter()
{
	bReplicates = true;
#if ENGINE_MINOR_VERSION < 24
	bReplicateMovement = true;
#else
	SetReplicatingMovement(true);
#endif

	GetCapsuleComponent()->InitCapsuleSize(38.0f, 38.0f);

	SphereComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereComponent"));
	SphereComponent->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'")));
	SphereComponent->SetMaterial(
		0, LoadObject<UMaterial>(nullptr, TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'")));
	SphereComponent->SetVisibility(true);
	SphereComponent->SetupAttachment(GetCapsuleComponent());

	FVector CameraLocation = FVector(300.0f, 0.0f, 75.0f);
	FRotator CameraRotation = FRotator::MakeFromEuler(FVector(0.0f, -10.0f, 180.0f));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
#if ENGINE_MINOR_VERSION < 24
	CameraComponent->bAbsoluteLocation = false;
	CameraComponent->bAbsoluteRotation = false;
	CameraComponent->RelativeLocation = CameraLocation;
	CameraComponent->RelativeRotation = CameraRotation;
#else
	CameraComponent->SetUsingAbsoluteLocation(false);
	CameraComponent->SetUsingAbsoluteRotation(false);
	CameraComponent->SetRelativeLocation(CameraLocation);
	CameraComponent->SetRelativeRotation(CameraRotation);
#endif
	CameraComponent->SetupAttachment(GetCapsuleComponent());
}

void ATestMovementCharacter::UpdateCameraLocationAndRotation_Implementation(FVector NewLocation, FRotator NewRotation)
{
	CameraComponent->SetRelativeLocation(NewLocation);
	CameraComponent->SetRelativeRotation(NewRotation);
}
