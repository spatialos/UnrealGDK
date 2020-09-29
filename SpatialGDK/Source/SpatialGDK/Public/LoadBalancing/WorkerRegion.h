// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
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
			  const float VerticalScale, const FString& WorkerName);

	UPROPERTY()
	UStaticMeshComponent* Mesh;

	UPROPERTY()
	UMaterialInstanceDynamic* MaterialBoundaryInstance;

	UPROPERTY()
	// UMaterialInstanceDynamic* MaterialTextInstance;
	UMaterial* MaterialTextInstance;

private:
	void SetOpacityAndEmissive(const float Opacity, const float Emissive);
	void SetHeight(const float Height);
	void SetPositionAndScale(const FBox2D& Extents, const float VerticalScale, bool bSetPosition, bool bSetScale);
	void SetColor(const FColor& Color);
	void TileWallWithWorkerText(const bool bTileX, const FBox2D& Extents, const float VerticalScale, const float C, float Offset,
								UMaterial* TextMaterial, UFont* TextFont, const FString& WorkerName, const float TextYaw);
	void CreateWorkerTextAtPosition(UMaterial* TextMaterial, UFont* TextFont, const float& VerticalScale, const FString& WorkerName,
									const float& PositionX, const float& PositionY, const float& PositionZ, const float& Yaw);
};
