// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PartiallyStablePathActor.h"

#include "Components/StaticMeshComponent.h"

void APartiallyStablePathActor::BeginPlay()
{
	Super::BeginPlay();

	Component = NewObject<UStaticMeshComponent>(this, "PartiallyStablePathComponent");
	Component->SetNetAddressable();
	Component->RegisterComponent();

	Component->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")));
	Component->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	Component->SetRelativeTransform(FTransform(FRotator(), FVector(0.0f, 0.0f, 50.0f), FVector(0.5f)));
}
