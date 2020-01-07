// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Math/Box2D.h"
#include "Math/Color.h"

#include "WorkerRegion.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWorkerRegion, Log, All)

UCLASS(NotPlaceable, NotBlueprintable)
class SPATIALGDK_API AWorkerRegion : public AActor
{
	GENERATED_BODY()

public:
	AWorkerRegion(const FObjectInitializer& ObjectInitializer);

	void Init(UMaterial* Material, const FColor& Color, const FBox2D& Extents);
	void SetOpacity(const float Opacity);
	void SetHeight(const float Height);

	UPROPERTY()
	UStaticMeshComponent *Mesh;

	UPROPERTY()
	UMaterialInstanceDynamic *MaterialInstance;

private:
	void SetExtents(const FBox2D& Extents);
	void SetColor(const FColor& Color);
};
