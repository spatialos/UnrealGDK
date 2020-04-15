// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpatialConstants.h"

#include "SpatialLayerManager.generated.h"

USTRUCT()
struct FWorkerType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "SpatialGDK")
	FName WorkerTypeName;

	FWorkerType() : WorkerTypeName(NAME_None)
	{
	}

	FWorkerType(FName InWorkerTypeName) : WorkerTypeName(InWorkerTypeName)
	{
	}
};

class SPATIALGDK_API SpatialLayerManager
{
private:
	TMap<TSoftClassPtr<AActor>, FName> ClassPathToLayer;

	TMap<FName, FName> LayerToWorkerType;

	FName DefaultWorkerType;

public:
	void Init();

	// Returns the first Layer that contains this, or a parent of this class,
	// or the default actor group, if no mapping is found.
	FName GetLayerForClass(TSubclassOf<AActor> Class);

	// Returns the Server worker type that is authoritative over the Layer
	// that contains this class (or parent class). Returns DefaultWorkerType
	// if no mapping is found.
	FName GetWorkerTypeForClass(TSubclassOf<AActor> Class);

	// Returns the Server worker type that is authoritative over this Layer.
	FName GetWorkerTypeForLayer(const FName& Layer) const;

	// Returns true if ActorA and ActorB are contained in Layers that are
	// on the same Server worker type.
	bool IsSameWorkerType(const AActor* ActorA, const AActor* ActorB);
};
