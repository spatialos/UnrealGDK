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

	void Init(UMaterial* Material, const FColor& Color, const FBox2D& Extents, const float VerticalScale, const FString& WorkerName);

	UPROPERTY()
	UStaticMeshComponent* Mesh;

	UPROPERTY()
	UMaterialInstanceDynamic* MaterialInstance;

	UPROPERTY()
	UTextRenderComponent* WorkerText;

private:
	void SetOpacity(const float Opacity);
	void SetHeight(const float Height);
	void SetPositionAndScale(const FBox2D& Extents, const float VerticalScale);
	void SetColor(const FColor& Color);
};
