// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/WorkerRegion.h"

#include "Engine/Canvas.h"
#include "Engine/EngineTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Runtime/Engine/Classes/Engine/CanvasRenderTarget2D.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

namespace
{
const float DEFAULT_WORKER_REGION_HEIGHT = 30.0f;
const float DEFAULT_WORKER_REGION_OPACITY = 0.7f;
const FString WORKER_REGION_ACTOR_NAME = TEXT("WorkerRegionCuboid");
const FName WORKER_REGION_MATERIAL_OPACITY_PARAM = TEXT("Opacity");
const FName WORKER_REGION_MATERIAL_COLOR_PARAM = TEXT("Color");
const FName WORKER_TEXT_MATERIAL_TP2D_PARAM = TEXT("TP2D");
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

void AWorkerRegion::Init(UMaterial* BackgroundMaterial, UMaterial* InCombinedMaterial, UFont* InWorkerInfoFont, const FColor& Color,
						 const FBox2D& Extents, const float VerticalScale, const FString& InWorkerInfo, const bool bInEditor)
{
	// Background translucent coloured worker material
	BackgroundMaterialInstance = UMaterialInstanceDynamic::Create(BackgroundMaterial, nullptr);

	if (bInEditor)
	{
		// In editor, display the basic boundary material
		Mesh->SetMaterial(0, BackgroundMaterialInstance);
	}
	else
	{
		// At runtime, setup the dynamic boundary material 
		CanvasRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), 1024, 1024);
		CanvasRenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &AWorkerRegion::DrawToCanvasRenderTarget);

		CombinedMaterial = InCombinedMaterial;
		WorkerInfoFont = InWorkerInfoFont;
		WorkerInfo = InWorkerInfo;
	}

	SetHeight(DEFAULT_WORKER_REGION_HEIGHT);
	SetOpacity(DEFAULT_WORKER_REGION_OPACITY);
	SetColor(Color);
	SetPositionAndScale(Mesh, Extents, true, VerticalScale);

	if (!bInEditor)
	{
		// At runtime, calls DrawToCanvasRenderTarget to render the dynamic boundary material
		CanvasRenderTarget->UpdateResource();
	}
}

// Render the dynamic boundary material with a translucent coloured background and worker information 
void AWorkerRegion::DrawToCanvasRenderTarget(UCanvas* Canvas, int32 Width, int32 Height)
{
	// Draw the worker background to the canvas
	Canvas->K2_DrawMaterial(BackgroundMaterialInstance, FVector2D(0, 0), FVector2D(Width, Height), FVector2D(0, 0));

	// Draw the worker information to the canvas
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(WorkerInfoFont, WorkerInfo, 0, 0, 1.0, 1.0);

	// Create a dynamic boundary material and attach it to this mesh
	CombinedMaterialInstance = UMaterialInstanceDynamic::Create(CombinedMaterial, nullptr);
	Mesh->SetMaterial(0, CombinedMaterialInstance);
	// Set the material parameter TP2D for the dynamic texture to use the canvas as input
	CombinedMaterialInstance->SetTextureParameterValue(WORKER_TEXT_MATERIAL_TP2D_PARAM, CanvasRenderTarget);
}

void AWorkerRegion::SetHeight(const float Height)
{
	const FVector CurrentLocation = GetActorLocation();
	SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, Height));
}

void AWorkerRegion::SetOpacity(const float Opacity)
{
	BackgroundMaterialInstance->SetScalarParameterValue(WORKER_REGION_MATERIAL_OPACITY_PARAM, Opacity);
}

void AWorkerRegion::SetPositionAndScale(UStaticMeshComponent* Wall, const FBox2D& Extents, bool bXAxis, const float VerticalScale)
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

	SetActorLocation(FVector(CenterX, CenterY, CurrentLocation.Z));
	SetActorScale3D(FVector(ScaleX, ScaleY, VerticalScale));
}

void AWorkerRegion::SetColor(const FColor& Color)
{
	BackgroundMaterialInstance->SetVectorParameterValue(WORKER_REGION_MATERIAL_COLOR_PARAM, Color);
}
