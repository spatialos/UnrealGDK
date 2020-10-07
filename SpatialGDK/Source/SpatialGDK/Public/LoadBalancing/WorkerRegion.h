// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "GameFramework/Actor.h"
#include "Math/Box2D.h"
#include "Math/Color.h"

#include "WorkerRegion.generated.h"

UCLASS(Transient, NotPlaceable, NotBlueprintable)
class SPATIALGDK_API AWorkerRegion : public AActor
{
	GENERATED_BODY()

public:
	AWorkerRegion(const FObjectInitializer& ObjectInitializer);

	void Init(UMaterial* BoundaryMaterial, UMaterial* TextMaterial, UFont* TextFont, const FColor& Color, const FBox2D& Extents,
			  const float VerticalScale, const FString& InWorkerInfo, const bool bInEditor);

	UPROPERTY()
	UStaticMeshComponent* Mesh;

	UPROPERTY()
	UMaterialInstanceDynamic* MaterialBoundaryInstance;

	UPROPERTY()
	UMaterialInstanceDynamic* MaterialTextInstance;

	UPROPERTY()
	UMaterial* TextMaterial;

	UPROPERTY()
	UCanvasRenderTarget2D* CanvasRenderTarget;

	UPROPERTY()
	UFont* WorkerInfoFont;

	UPROPERTY()
	FString WorkerInfo;

	// UPROPERTY()
	// UTexture2D* WorkerBoundaryTexture;

	UFUNCTION()
	void DrawToCanvasRenderTarget(UCanvas* Canvas, int32 Width, int32 Height);

private:
	void SetOpacityAndEmissive(const float Opacity, const float Emissive);
	void SetHeight(const float Height);
	void SetPositionAndScale(UStaticMeshComponent* Wall, const FBox2D& Extents, bool bXAxis, const float VerticalScale);
	void SetColor(const FColor& Color);
};
