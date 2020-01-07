// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/WorkerRegion.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY(LogWorkerRegion);

namespace
{
	const float DEFAULT_WORKER_REGION_HEIGHT = 30.0f;
	const float DEFAULT_WORKER_REGION_OPACITY = 0.7f;
	const FString WORKER_REGION_ACTOR_NAME = TEXT("WorkerRegionPlane");
	const FName WORKER_REGION_MATERIAL_OPACITY_PARAM = TEXT("Opacity");
	const FName WORKER_REGION_MATERIAL_COLOR_PARAM = TEXT("Color");
	const FString PLANE_MESH_PATH = TEXT("/Engine/BasicShapes/Plane.Plane");
}

AWorkerRegion::AWorkerRegion(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, *WORKER_REGION_ACTOR_NAME);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(*PLANE_MESH_PATH);
	Mesh->SetStaticMesh(PlaneAsset.Object);
	SetRootComponent(Mesh);
}

void AWorkerRegion::Init(UMaterial* Material, const FColor& Color, const FBox2D& Extents)
{
	SetHeight(DEFAULT_WORKER_REGION_HEIGHT);

	MaterialInstance = UMaterialInstanceDynamic::Create(Material, nullptr);
	Mesh->SetMaterial(0, MaterialInstance);
	SetOpacity(DEFAULT_WORKER_REGION_OPACITY);
	SetColor(Color);
	this->SetExtents(Extents);
}

void AWorkerRegion::SetHeight(const float Height)
{
	const FVector CurrentLocation = this->GetActorLocation();
	SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, Height));
}

void AWorkerRegion::SetOpacity(const float Opacity)
{
	MaterialInstance->SetScalarParameterValue(WORKER_REGION_MATERIAL_OPACITY_PARAM, Opacity);
}

void AWorkerRegion::SetExtents(const FBox2D& Extents)
{
	const FVector CurrentLocation = this->GetActorLocation();

	const float& MinX = Extents.Min.X;
	const float& MaxX = Extents.Max.X;
	const float& MinY = Extents.Min.Y;
	const float& MaxY = Extents.Max.Y;

	float CenterX = MinX + (MaxX - MinX) / 2;
	float CenterY = MinY + (MaxY - MinY) / 2;
	float ScaleX = (MaxX - MinX) / 100;
	float ScaleY = (MaxY - MinY) / 100;

	SetActorLocation(FVector(CenterX, CenterY,  CurrentLocation.Z));
	SetActorScale3D(FVector(ScaleX, ScaleY,  1));
}

void AWorkerRegion::SetColor(const FColor& Color)
{
	MaterialInstance->SetVectorParameterValue(WORKER_REGION_MATERIAL_COLOR_PARAM, Color);
}
