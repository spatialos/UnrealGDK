// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Math/Box2D.h"
#include "Math/Color.h"

#include "WorkerRegion.generated.h"

class UCanvas;
class UCanvasRenderTarget2D;

UCLASS(Transient, NotPlaceable, NotBlueprintable)
class SPATIALGDK_API AWorkerRegion : public AActor
{
	GENERATED_BODY()

public:
	AWorkerRegion(const FObjectInitializer& ObjectInitializer);

	void Init(UMaterial* BackgroundMaterial, UMaterial* InCombinedMaterial, UFont* InWorkerInfoFont, const FColor& Color, const float Opacity,
			  const FBox2D& Extents, const float Height, const float VerticalScale, const FString& InWorkerInfo, const bool bInEditor);

	UPROPERTY()
	UStaticMeshComponent* Mesh;

	UPROPERTY()
	UMaterialInstanceDynamic* BackgroundMaterialInstance;

	UPROPERTY()
	UMaterialInstanceDynamic* CombinedMaterialInstance;

	UPROPERTY()
	UMaterial* CombinedMaterial;

	UPROPERTY()
	UCanvasRenderTarget2D* CanvasRenderTarget;

	UPROPERTY()
	UFont* WorkerInfoFont;

	UPROPERTY()
	FString WorkerInfo;

	UFUNCTION()
	void DrawToCanvasRenderTarget(UCanvas* Canvas, int32 Width, int32 Height);

private:
	void SetOpacity(const float Opacity);
	void SetHeight(const float Height);
	void SetPositionAndScale(UStaticMeshComponent* Wall, const FBox2D& Extents, bool bXAxis, const float VerticalScale);
	void SetColor(const FColor& Color);
};
