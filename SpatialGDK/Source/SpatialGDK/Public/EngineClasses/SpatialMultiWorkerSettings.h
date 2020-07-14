// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "Utils/LayerInfo.h"

#include "Templates/SubclassOf.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"

#include "SpatialMultiWorkerSettings.generated.h"

class UAbstractLockingPolicy;

namespace SpatialGDK
{
const FLayerInfo DefaultLayerInfo = { SpatialConstants::DefaultLayer, {AActor::StaticClass()}, USingleWorkerStrategy::StaticClass()};
}

UCLASS(NotBlueprintable)
class SPATIALGDK_API UAbstractSpatialMultiWorkerSettings : public UObject
{
	GENERATED_BODY()

public:
	UAbstractSpatialMultiWorkerSettings() {};

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	UAbstractSpatialMultiWorkerSettings(TArray<FLayerInfo> InWorkerLayers, TSubclassOf<UAbstractLockingPolicy> InLockingPolicy)
		: WorkerLayers(InWorkerLayers)
		, LockingPolicy(InLockingPolicy) {}

public:
	uint32 GetMinimumRequiredWorkerCount() const;

	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
		TArray<FLayerInfo> WorkerLayers;

	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
		TSubclassOf<UAbstractLockingPolicy> LockingPolicy;

private:
	void ValidateNonEmptyWorkerLayers();
	void ValidateSomeLayerHasActorClass();
	void ValidateNoActorClassesDuplicatedAmongLayers();
	void ValidateAllLayersHaveUniqueNonemptyNames();
	void ValidateAllLayersHaveLoadBalancingStrategy();
	void ValidateLockingPolicyIsSet();
};

UCLASS(Blueprintable)
class SPATIALGDK_API USpatialMultiWorkerSettings : public UAbstractSpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	USpatialMultiWorkerSettings()
		: Super({SpatialGDK::DefaultLayerInfo}, UOwnershipLockingPolicy::StaticClass())
	{}
};
