// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/WorkerRegion.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY(LogWorkerRegion);

const float AWorkerRegion::DEFAULT_WORKER_REGION_HEIGHT = 10;
const float AWorkerRegion::DEFAULT_WORKER_REGION_OPACITY = 0.7;

AWorkerRegion::AWorkerRegion(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("WorkerRegionPlane"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
	Mesh->SetStaticMesh(PlaneAsset.Object);
	SetRootComponent(Mesh);
}

void AWorkerRegion::Init(UMaterial* Material, FColor Color, FBox2D Extents)
{
	this->SetHeight(AWorkerRegion::DEFAULT_WORKER_REGION_HEIGHT);

	MaterialInstance = UMaterialInstanceDynamic::Create(Material, nullptr);
	Mesh->SetMaterial(0, MaterialInstance);
	this->SetOpacity(AWorkerRegion::DEFAULT_WORKER_REGION_OPACITY);
	this->SetColor(Color);
	this->SetExtents(Extents);
}

void AWorkerRegion::SetHeight(float Height)
{
	const FVector CurrentLocation = this->GetActorLocation();
	this->SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, Height));
}

void AWorkerRegion::SetOpacity(float Opacity)
{
	MaterialInstance->SetScalarParameterValue(TEXT("Opacity"), Opacity);
}

void AWorkerRegion::SetExtents(FBox2D Extents)
{
	const FVector CurrentLocation = this->GetActorLocation();
	this->SetActorLocation(FVector(Extents.Min.X + (Extents.Max.X - Extents.Min.X) / 2, Extents.Min.Y + (Extents.Max.Y - Extents.Min.Y) / 2,  CurrentLocation.Z));
	this->SetActorScale3D(FVector((Extents.Max.X - Extents.Min.X) / 100, (Extents.Max.Y - Extents.Min.Y) / 100,  1));
}

void AWorkerRegion::SetColor(FColor Color)
{
	MaterialInstance->SetVectorParameterValue(TEXT("Color"), Color);
}
