// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "Utils/LayerInfo.h"

#include "Templates/SubclassOf.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"

#include "SpatialMultiWorkerSettings.generated.h"

class UAbstractLockingPolicy;

UCLASS(NotBlueprintable)
class SPATIALGDK_API UAbstractSpatialMultiWorkerSettings : public UObject
{
	GENERATED_BODY()

public:
	UAbstractSpatialMultiWorkerSettings(TMap<FName, FLayerInfo> InWorkerLayers, TSubclassOf<UAbstractLockingPolicy> InLockingPolicy)
    : WorkerLayers(InWorkerLayers)
    , LockingPolicy(InLockingPolicy)
	{
	}

	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
		TMap<FName, FLayerInfo> WorkerLayers;

	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
		TSubclassOf<UAbstractLockingPolicy> LockingPolicy;
};

UCLASS(Blueprintable)
class SPATIALGDK_API USpatialMultiWorkerSettings : public UAbstractSpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	const auto DefaultLayer = std::initializer_list<TPairInitializer<const FName&, const FLayerInfo&>>({
		TPairInitializer<const FName&, const FLayerInfo&>(TEXT("Default"), {"Default", {AActor::StaticClass()}, UGridBasedLBStrategy::StaticClass()})
	});

	USpatialMultiWorkerSettings() : Super(DefaultLayer, UOwnershipLockingPolicy::StaticClass()) {}
};
