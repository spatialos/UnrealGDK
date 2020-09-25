// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/WorkerRegion.h"

#include "Engine/EngineTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

namespace
{
const float DEFAULT_WORKER_REGION_HEIGHT = 30.0f;
const float DEFAULT_WORKER_REGION_OPACITY = 0.7f;
const FString WORKER_REGION_ACTOR_NAME = TEXT("WorkerRegionCuboid");
const FName WORKER_REGION_MATERIAL_OPACITY_PARAM = TEXT("Opacity");
const FName WORKER_REGION_MATERIAL_COLOR_PARAM = TEXT("Color");
const FString CUBE_MESH_PATH = TEXT("/Engine/BasicShapes/Cube.Cube");
} // namespace

AWorkerRegion::AWorkerRegion(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Mesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, *WORKER_REGION_ACTOR_NAME);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(*CUBE_MESH_PATH);
	Mesh->SetStaticMesh(CubeAsset.Object);
	SetRootComponent(Mesh);
}

void AWorkerRegion::Init(UMaterial* BoundaryMaterial, UMaterial* TextMaterial, UFont* TextFont, const FColor& Color, const FBox2D& Extents,
						 const float VerticalScale, const FString& WorkerName)
{
	MaterialBoundaryInstance = UMaterialInstanceDynamic::Create(BoundaryMaterial, nullptr);
	// TODO: create this statically in editor as will not change?
	MaterialTextInstance = UMaterialInstanceDynamic::Create(TextMaterial, nullptr);

	SetHeight(DEFAULT_WORKER_REGION_HEIGHT);
	SetOpacity(DEFAULT_WORKER_REGION_OPACITY);

	Mesh->SetMaterial(0, MaterialBoundaryInstance);

	SetColor(Color);
	SetPositionAndScale(Extents, VerticalScale, false, true);

	// Tile horizontally
	int xCount = Extents.GetSize().X / 250;
	// Tile vertically
	int zCount = (int)VerticalScale; // * 2; // Add the x2 for denser vertical tiling
	const float C = 50;
	float D = C;
	for (int xi = 0; xi < xCount - 1; xi++)
	{
		D -= 100.f / xCount;
	}
	const float A = 0;
	float zDiff = 100.f / zCount; // VerticalScale;
	for (int xi = 0; xi < xCount; xi++)
	{
		float B = xCount - 1;
		float xPos = (xi - A) / (B - A) * (D - C) + C;
		for (int zi = 0; zi < zCount; zi++)
		{
			// Using exactly 50 causes the text to flicker so used nearly 50 instead
			float zPosition = (zi * zDiff) - ((zCount * zDiff) / 2.f);
			CreateWorkerTextAtPosition(TextMaterial, TextFont, VerticalScale, WorkerName, xPos, 49.999, zPosition);
		}
	}

	SetPositionAndScale(Extents, VerticalScale, true, false);
}

void AWorkerRegion::CreateWorkerTextAtPosition(UMaterial* TextMaterial, UFont* TextFont, const float& VerticalScale,
											   const FString& WorkerName, const float& PositionX, const float& PositionY,
											   const float& PositionZ)
{
	// Create dynamic worker name text on boundary wall
	UTextRenderComponent* WorkerText = NewObject<UTextRenderComponent>(this);
	// WorkerText->SetFont(TextFont); // Only works independantly of setting the material instance
	WorkerText->SetTextMaterial(MaterialTextInstance);
	FRotator NewRotation = FRotator(0, 270, 0);
	FQuat QuatRotation = FQuat(NewRotation);
	WorkerText->SetWorldRotation(QuatRotation);
	WorkerText->SetRelativeLocation(FVector(PositionX, PositionY, PositionZ));
	WorkerText->SetTextRenderColor(FColor::White);
	WorkerText->SetText((TEXT("Worker boundary %s"), WorkerName));
	WorkerText->SetXScale(1.f);
	WorkerText->SetYScale(1.f);
	WorkerText->SetWorldSize(10);
	FAttachmentTransformRules TextTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld,
												 false);
	WorkerText->AttachToComponent(this->GetRootComponent(), TextTransformRules);
	WorkerText->RegisterComponent();
}

void AWorkerRegion::SetHeight(const float Height)
{
	const FVector CurrentLocation = GetActorLocation();
	SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, Height));
}

void AWorkerRegion::SetOpacity(const float Opacity)
{
	MaterialBoundaryInstance->SetScalarParameterValue(WORKER_REGION_MATERIAL_OPACITY_PARAM, Opacity);
	MaterialTextInstance->SetScalarParameterValue(WORKER_REGION_MATERIAL_OPACITY_PARAM, Opacity);
}

void AWorkerRegion::SetPositionAndScale(const FBox2D& Extents, const float VerticalScale, bool bSetPosition, bool bSetScale)
{
	const FVector CurrentLocation = GetActorLocation();

	const float MinX = Extents.Min.X;
	const float MaxX = Extents.Max.X;
	const float MinY = Extents.Min.Y;
	const float MaxY = Extents.Max.Y;

	const float CenterX = MinX + (MaxX - MinX) / 2;
	const float CenterY = MinY + (MaxY - MinY) / 2;
	const float ScaleX = (MaxX - MinX) / 100;
	const float ScaleY = (MaxY - MinY) / 100;

	if (bSetPosition)
	{
		SetActorLocation(FVector(CenterX, CenterY, CurrentLocation.Z));
	}
	if (bSetScale)
	{
		SetActorScale3D(FVector(ScaleX, ScaleY, VerticalScale));
	}
}

void AWorkerRegion::SetColor(const FColor& Color)
{
	MaterialBoundaryInstance->SetVectorParameterValue(WORKER_REGION_MATERIAL_COLOR_PARAM, Color);
}
