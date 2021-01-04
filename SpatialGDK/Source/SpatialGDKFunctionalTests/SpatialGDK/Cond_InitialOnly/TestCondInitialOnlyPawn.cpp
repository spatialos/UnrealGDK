// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestCondInitialOnlyPawn.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/Classes/Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Materials/Material.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTestCondInitialOnlyPawn);

ATestCondInitialOnlyPawn::ATestCondInitialOnlyPawn()
{
	SphereComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereComponent"));
	SphereComponent->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'")));
	SphereComponent->SetMaterial(
		0, LoadObject<UMaterial>(nullptr, TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'")));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComponent->SetVisibility(true);
	RootComponent = SphereComponent;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);

	FVector CameraLocation = FVector(300.0f, 0.0f, 75.0f);
	FRotator CameraRotation = FRotator::MakeFromEuler(FVector(0.0f, -10.0f, 180.0f));

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

	TestValue = 1;
}

void ATestCondInitialOnlyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATestCondInitialOnlyPawn, TestValue, COND_InitialOnly);
}

void ATestCondInitialOnlyPawn::SetTestValue(int32 Value)
{
	TestValue = Value;
	UE_LOG(LogTestCondInitialOnlyPawn, Log, TEXT("%s SetTestValue:%d"), *GetName(), TestValue);
}

int32 ATestCondInitialOnlyPawn::GetTestValue()
{
	return TestValue;
}
