// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PartiallyStablePathActor.h"

#include "Components/StaticMeshComponent.h"

void APartiallyStablePathActor::BeginPlay()
{
	Super::BeginPlay();

	DynamicComponent = NewObject<UStaticMeshComponent>(this, "PartiallyStablePathComponent");
	DynamicComponent->SetNetAddressable();
	DynamicComponent->RegisterComponent();

	DynamicComponent->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")));
	DynamicComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	DynamicComponent->SetRelativeTransform(FTransform(FRotator(), FVector(0.0f, 0.0f, 50.0f), FVector(0.5f)));
}
