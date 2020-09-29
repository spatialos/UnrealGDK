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
const float DEFAULT_WORKER_TEXT_EMISSIVE = 0.2f;
const FString WORKER_REGION_ACTOR_NAME = TEXT("WorkerRegionCuboid");
const FName WORKER_REGION_MATERIAL_OPACITY_PARAM = TEXT("Opacity");
const FName WORKER_REGION_MATERIAL_COLOR_PARAM = TEXT("Color");
const FName WORKER_TEXT_MATERIAL_EMMISIVE_PARAM = TEXT("Emissive");
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
	//MaterialTextInstance = UMaterialInstanceDynamic::Create(TextMaterial, nullptr);
	MaterialTextInstance = TextMaterial;

	SetHeight(DEFAULT_WORKER_REGION_HEIGHT);
	SetOpacityAndEmissive(DEFAULT_WORKER_REGION_OPACITY, DEFAULT_WORKER_TEXT_EMISSIVE);

	Mesh->SetMaterial(0, MaterialBoundaryInstance);

	SetColor(Color);
	SetPositionAndScale(Extents, VerticalScale, false, true);

	// North wall
	// Note: using an offset of 50 causes the text to flicker so used just less than 50 instead
	TileWallWithWorkerText(false, Extents, VerticalScale, 0, 49.999f, TextMaterial, TextFont, WorkerName, 180);
	// East wall
	TileWallWithWorkerText(true, Extents, VerticalScale, 50, 49.999, TextMaterial, TextFont, WorkerName, 270);
	// South wall
	TileWallWithWorkerText(false, Extents, VerticalScale, 50, -49.999f, TextMaterial, TextFont, WorkerName, 0);
	// West wall
	TileWallWithWorkerText(true, Extents, VerticalScale, 25, -49.999f, TextMaterial, TextFont, WorkerName, 90);

	SetPositionAndScale(Extents, VerticalScale, true, false);
}

void AWorkerRegion::TileWallWithWorkerText(const bool bTileXAxis, const FBox2D& Extents, const float VerticalScale, const float TileOffset,
										   float CentreOffset, UMaterial* TextMaterial, UFont* TextFont, const FString& WorkerName,
										   const float Yaw)
{
	int HorizontalTileCount = 0;

	if (bTileXAxis)
	{
		// Tile on x-axis
		HorizontalTileCount = Extents.GetSize().X / 250;
	}
	else
	{
		// Tile on y-axis
		HorizontalTileCount = Extents.GetSize().Y / 250;
	}

	// TODO : make density a parameter
	int VerticalTileCount = (int)VerticalScale; // * 2; // Add the x2 for denser vertical tiling - remove for sparser

	float FinalTileOffset = TileOffset;
	for (int i = 0; i < HorizontalTileCount - 1; i++)
	{
		FinalTileOffset -= 100.f / HorizontalTileCount;
	}

	// const float A = 0;
	float SpacingZ = 100.f / VerticalTileCount; // VerticalScale;
	float PositionX = 0;
	float PositionY = 0;
	// Tile horizontally
	for (int i = 0; i < HorizontalTileCount; i++)
	{
		// float B = HorizontalTileCount - 1;
		float Position = i / (float)(HorizontalTileCount - 1) * (FinalTileOffset - TileOffset) + TileOffset;
		if (bTileXAxis)
		{
			// Tile on x-axis
			PositionX = Position;
			PositionY = CentreOffset;
		}
		else
		{
			// Tile on y-axis
			PositionX = CentreOffset;
			PositionY = Position;
		}

		// Tile vertically
		for (int zi = 0; zi < VerticalTileCount; zi++)
		{
			float PositionZ = (zi * SpacingZ) - ((VerticalTileCount * SpacingZ) / 2.f);
			CreateWorkerTextAtPosition(TextMaterial, TextFont, VerticalScale, WorkerName, PositionX, PositionY, PositionZ, Yaw);
		}
	}
}

void AWorkerRegion::CreateWorkerTextAtPosition(UMaterial* TextMaterial, UFont* TextFont, const float& VerticalScale,
											   const FString& WorkerName, const float& PositionX, const float& PositionY,
											   const float& PositionZ, const float& Yaw)
{
	// Create dynamic worker name text on boundary wall
	UTextRenderComponent* WorkerText = NewObject<UTextRenderComponent>(this);
	WorkerText->SetFont(TextFont); // Only works independantly of setting the material instance
	WorkerText->SetTextMaterial(MaterialTextInstance);
	FRotator NewRotation = FRotator(0, Yaw, 0);
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
