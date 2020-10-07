// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/WorkerRegion.h"

#include "Engine/Canvas.h"
#include "Engine/EngineTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectGlobals.h"

#include "Logging/LogMacros.h"
#include "Runtime/Engine/Classes/Engine/CanvasRenderTarget2D.h"

namespace
{
const float DEFAULT_WORKER_REGION_HEIGHT = 30.0f;
const float DEFAULT_WORKER_REGION_OPACITY = 0.7f;
const float DEFAULT_WORKER_TEXT_EMISSIVE = 0.2f;
const FString WORKER_REGION_ACTOR_NAME = TEXT("WorkerRegionCuboid");
const FName WORKER_REGION_MATERIAL_OPACITY_PARAM = TEXT("Opacity");
const FName WORKER_REGION_MATERIAL_COLOR_PARAM = TEXT("Color");
const FName WORKER_TEXT_MATERIAL_EMMISIVE_PARAM = TEXT("Emissive");
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

void AWorkerRegion::Init(UMaterial* BoundaryMaterial, UMaterial* InTextMaterial, UFont* InWorkerInfoFont, const FColor& Color,
						 const FBox2D& Extents, const float VerticalScale, const FString& InWorkerInfo, const bool bInEditor)
{
	MaterialBoundaryInstance = UMaterialInstanceDynamic::Create(BoundaryMaterial, nullptr);

	if (bInEditor)
	{
		// translucent boundary
		Mesh->SetMaterial(0, MaterialBoundaryInstance); 
	}
	else
	{
		// dynamic boundary material
		CanvasRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), 1024, 1024);
		CanvasRenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &AWorkerRegion::DrawToCanvasRenderTarget);

		TextMaterial = InTextMaterial;
		WorkerInfoFont = InWorkerInfoFont;
		WorkerInfo = InWorkerInfo;
	}

	SetHeight(DEFAULT_WORKER_REGION_HEIGHT);
	SetOpacityAndEmissive(DEFAULT_WORKER_REGION_OPACITY, DEFAULT_WORKER_TEXT_EMISSIVE);

	//Mesh->SetMaterial(0, MaterialBoundaryInstance); // Translucent neighbour boundary

	SetColor(Color);

	SetPositionAndScale(Mesh, Extents, true, VerticalScale);

	if (!bInEditor)
	{
		CanvasRenderTarget->UpdateResource();
	}
}

void AWorkerRegion::DrawToCanvasRenderTarget(UCanvas* Canvas, int32 Width, int32 Height)
{
	// Draw the worker border material to the canvas
	Canvas->K2_DrawMaterial(MaterialBoundaryInstance, FVector2D(0, 0), FVector2D(Width, Height), FVector2D(0, 0));
	// Draw the worker information to the canvas
	Canvas->SetDrawColor(FColor::White);
	Canvas->DrawText(WorkerInfoFont, WorkerInfo, 0, 0, 1.0, 1.0);

	// Create a dynamic material and attach it to this mesh
	MaterialTextInstance = UMaterialInstanceDynamic::Create(TextMaterial, nullptr);
	Mesh->SetMaterial(0, MaterialTextInstance);
	// Set the material parameter TP2D for the dynamic texture to use the canvas as input
	MaterialTextInstance->SetTextureParameterValue(WORKER_TEXT_MATERIAL_TP2D_PARAM, CanvasRenderTarget);
}

void AWorkerRegion::SetHeight(const float Height)
{
	const FVector CurrentLocation = GetActorLocation();
	SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, Height));
}

void AWorkerRegion::SetOpacityAndEmissive(const float Opacity, const float Emissive)
{
	MaterialBoundaryInstance->SetScalarParameterValue(WORKER_REGION_MATERIAL_OPACITY_PARAM, Opacity);
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
	MaterialBoundaryInstance->SetVectorParameterValue(WORKER_REGION_MATERIAL_COLOR_PARAM, Color);
}
