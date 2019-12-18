// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Math/Box2D.h"
#include "Math/Color.h"

#include "WorkerRegion.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWorkerRegion, Log, All)

UCLASS()
class SPATIALGDK_API AWorkerRegion : public AActor
{
	GENERATED_BODY()

public:
	AWorkerRegion(const FObjectInitializer& ObjectInitializer);

	void Init(UMaterial* Material, FColor Color, FBox2D Extents);
	void SetOpacity(float Opacity);
	void SetHeight(float Height);

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent *Mesh;

	UPROPERTY(EditAnywhere)
	UMaterialInstanceDynamic *MaterialInstance;

	static const float DEFAULT_WORKER_REGION_HEIGHT;
	static const float DEFAULT_WORKER_REGION_OPACITY;

private:
	void SetExtents(FBox2D Extents);
	void SetColor(FColor Color);
};
