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
const float DEFAULT_WORKER_REGION_OPACITY = 1.0f;
// 0.7f;
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
	// Create canvas for dynamic worker text
	// Create dynamic worker name text on boundary wall

	// static boundary material
	// MaterialTextInstance = TextMaterial;
	MaterialBoundaryInstance = UMaterialInstanceDynamic::Create(BoundaryMaterial, nullptr);

	if (!bInEditor)
	{
		// dynamic boundary material
		CanvasRenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(this, UCanvasRenderTarget2D::StaticClass(), 1024, 1024);
		CanvasRenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &AWorkerRegion::DrawToCanvasRenderTarget);
		// CanvasRenderTarget->UpdateResource();

		// MaterialTextInstance = UMaterialInstanceDynamic::Create(TextMaterial, nullptr); // dynamic material
		TextMaterial = InTextMaterial;
		WorkerInfoFont = InWorkerInfoFont;
		WorkerInfo = InWorkerInfo;
	}

	// MaterialTextInstance = UMaterialInstanceDynamic::Create(TextMaterial, nullptr); // dynamic material
	////MaterialTextInstance = TextMaterial; // static material
	// WorkerInfoFont = InWorkerInfoFont;
	// WorkerInfo = InWorkerInfo;

	SetHeight(DEFAULT_WORKER_REGION_HEIGHT);
	SetOpacityAndEmissive(DEFAULT_WORKER_REGION_OPACITY, DEFAULT_WORKER_TEXT_EMISSIVE);

	// if (bInEditor)
	//{
	Mesh->SetMaterial(0, MaterialBoundaryInstance); // Translucent neighbour boundary
	//}
	// else
	//{
	// Mesh->SetMaterial(0, MaterialTextInstance); // Within worker text
	// MaterialTextInstance->SetTextureParameterValue(WORKER_TEXT_MATERIAL_TP2D_PARAM, CanvasRenderTarget);
	//}

	SetColor(Color);
	SetPositionAndScale(Extents, VerticalScale, false, true);

	//// North wall
	//// Note: using an offset of 50 causes the text to flicker so used just less than 50 instead
	// UE_LOG(LogTemp, Warning, TEXT("North wall"));
	// TileWallWithWorkerText(false, Extents, VerticalScale, -50, 49.999f, TextMaterial, TextFont, WorkerInfo, 180);
	//// East wall
	// UE_LOG(LogTemp, Warning, TEXT("East wall"));
	// TileWallWithWorkerText(true, Extents, VerticalScale, 50, 49.999, TextMaterial, TextFont, WorkerInfo, 270);
	//// South wall
	// UE_LOG(LogTemp, Warning, TEXT("South wall"));
	// TileWallWithWorkerText(false, Extents, VerticalScale, 50, -49.999f, TextMaterial, TextFont, WorkerInfo, 0);
	//// West wall
	// UE_LOG(LogTemp, Warning, TEXT("West wall"));
	// TileWallWithWorkerText(true, Extents, VerticalScale, -50, -49.999f, TextMaterial, TextFont, WorkerInfo, 90);

	SetPositionAndScale(Extents, VerticalScale, true, false);

	if (!bInEditor)
	{
		CanvasRenderTarget->UpdateResource();
	}
}

void AWorkerRegion::DrawToCanvasRenderTarget(UCanvas* Canvas, int32 Width, int32 Height)
{
	// Draw text
	// Canvas->SetDrawColor(0, 0, 0, 0); // sets colour of text not background
	// MaterialBoundaryInstance->SetScalarParameterValue(WORKER_REGION_MATERIAL_OPACITY_PARAM, DEFAULT_WORKER_REGION_OPACITY);
	// MaterialBoundaryInstance->SetScalarParameterValue(WORKER_REGION_MATERIAL_OPACITY_PARAM, 1.0);
	Canvas->K2_DrawMaterial(MaterialBoundaryInstance, FVector2D(0, 0), FVector2D(Width / 2.0, Height / 2.0), FVector2D(0, 0));
	Canvas->DrawText(WorkerInfoFont, WorkerInfo, 15, 15, 1.0, 1.0);

	MaterialTextInstance = UMaterialInstanceDynamic::Create(TextMaterial, nullptr); // dynamic material
	// MaterialTextInstance = UMaterialInstanceDynamic::Create(MaterialBoundaryInstance, nullptr); // dynamic material
	Mesh->SetMaterial(0, MaterialTextInstance); // Within worker text
	MaterialTextInstance->SetTextureParameterValue(WORKER_TEXT_MATERIAL_TP2D_PARAM, CanvasRenderTarget);
}

// void AWorkerRegion::TileWallWithWorkerText(const bool bTileXAxis, const FBox2D& Extents, const float VerticalScale, const float
// TileOffset, 										   const float CentreOffset, UMaterial* TextMaterial, UFont* TextFont, const FString& InWorkerInfo, 										   const float Yaw)
//{
//	int HorizontalTileCount = 0;
//	if (bTileXAxis)
//	{
//		// Tile on x-axis
//		HorizontalTileCount = Extents.GetSize().X / 500;
//		UE_LOG(LogTemp, Warning, TEXT("Extents x %f"), Extents.GetSize().X);
//	}
//	else
//	{
//		// Tile on y-axis
//		HorizontalTileCount = Extents.GetSize().Y / 500;
//		UE_LOG(LogTemp, Warning, TEXT("Extents y %f"), Extents.GetSize().Y);
//	}
//	UE_LOG(LogTemp, Warning, TEXT("HorizontalTileCount %d"), HorizontalTileCount);
//
//	// TODO : make density a parameter
//	int VerticalTileCount = (int)(VerticalScale * 0.5); // * 2; // Add the x2 for denser vertical tiling - 0.5 for sparser
//
//	float FinalTileOffset = TileOffset * -1;
//	float TileDifference = 100.f / (float)HorizontalTileCount;
//	float StartTileOffset = TileOffset;
//	if (TileOffset > 0)
//	{
//		StartTileOffset -= TileDifference / 2.0f;
//	}
//	else
//	{
//		StartTileOffset += TileDifference / 2.0f;
//	}
//
//	// UE_LOG(LogTemp, Warning, TEXT("TileOffset %f"), TileOffset);
//	// UE_LOG(LogTemp, Warning, TEXT("StartTileOffset %f"), StartTileOffset);
//	// UE_LOG(LogTemp, Warning, TEXT("FinalTileOffset %f"), FinalTileOffset);
//	float SpacingZ = 100.f / (float)VerticalTileCount;
//	float PositionX = 0;
//	float PositionY = 0;
//	// Tile horizontally
//	for (int i = 0; i < HorizontalTileCount; i++)
//	{
//		// UE_LOG(LogTemp, Warning, TEXT("i %d"), i);
//
//		float Position = 0;
//
//		if (TileOffset > 0)
//		{
//			Position = StartTileOffset - i * TileDifference;
//		}
//		else
//		{
//			Position = StartTileOffset + i * TileDifference;
//		}
//		if (bTileXAxis)
//		{
//			// Tile on x-axis
//			PositionX = Position;
//			PositionY = CentreOffset;
//		}
//		else
//		{
//			// Tile on y-axis
//			PositionX = CentreOffset;
//			PositionY = Position;
//		}
//
//		// Tile vertically
//		for (int zi = 0; zi < VerticalTileCount; zi++)
//		{
//			float PositionZ = (zi * SpacingZ) - ((VerticalTileCount * SpacingZ) / 2.f);
//			// UE_LOG(LogTemp, Warning, TEXT("Create at %f,%f,%f"), PositionX, PositionY, PositionZ);
//			CreateWorkerTextAtPosition(TextMaterial, TextFont, VerticalScale, WorkerInfo, PositionX, PositionY, PositionZ, Yaw);
//		}
//	}
//}
//
// void AWorkerRegion::CreateWorkerTextAtPosition(UMaterial* TextMaterial, UFont* TextFont, const float VerticalScale,
//											   const FString& WorkerInfo, const float PositionX, const float PositionY,
//											   const float PositionZ, const float Yaw)
//{
//	// Create dynamic worker name text on boundary wall
//	UTextRenderComponent* WorkerText = NewObject<UTextRenderComponent>(this);
//	WorkerText->SetFont(TextFont);
//	WorkerText->SetHorizontalAlignment(EHTA_Center);
//	WorkerText->SetHorizSpacingAdjust(2.0);
//	WorkerText->SetTextMaterial(MaterialTextInstance);
//	FRotator NewRotation = FRotator(0, Yaw, 0);
//	FQuat QuatRotation = FQuat(NewRotation);
//	WorkerText->SetWorldRotation(QuatRotation);
//	WorkerText->SetRelativeLocation(FVector(PositionX, PositionY, PositionZ));
//	WorkerText->SetTextRenderColor(FColor::White);
//	WorkerText->SetText(WorkerInfo);
//	WorkerText->SetXScale(1.f);
//	WorkerText->SetYScale(1.f);
//	WorkerText->SetWorldSize(10);
//	FAttachmentTransformRules TextTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld,
//												 false);
//	WorkerText->AttachToComponent(this->GetRootComponent(), TextTransformRules);
//	WorkerText->RegisterComponent();
//}

void AWorkerRegion::SetHeight(const float Height)
{
	const FVector CurrentLocation = GetActorLocation();
	SetActorLocation(FVector(CurrentLocation.X, CurrentLocation.Y, Height));
}

void AWorkerRegion::SetOpacityAndEmissive(const float Opacity, const float Emissive)
{
	MaterialBoundaryInstance->SetScalarParameterValue(WORKER_REGION_MATERIAL_OPACITY_PARAM, Opacity);
	//	MaterialTextInstance->SetScalarParameterValue(WORKER_REGION_MATERIAL_OPACITY_PARAM, Opacity);
	//	MaterialTextInstance->SetScalarParameterValue(WORKER_TEXT_MATERIAL_EMMISIVE_PARAM, Emissive);
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
