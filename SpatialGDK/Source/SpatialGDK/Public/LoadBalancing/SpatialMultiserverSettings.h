// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "SpatialConstants.h"
#include "Utils/LayerInfo.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "SpatialMultiserverSettings.generated.h"

class UAbstractLockingPolicy;

UCLASS(NotBlueprintable)
class SPATIALGDK_API UAbstractSpatialMultiserverSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UAbstractSpatialMultiserverSettings() {}

protected:
	UAbstractSpatialMultiserverSettings(TArray<FLayerInfo> InWorkerLayers, TSubclassOf<UAbstractLockingPolicy> InLockingPolicy)
		: WorkerLayers(InWorkerLayers)
		, LockingPolicy(InLockingPolicy) {}

public:
#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	uint32 GetMinimumRequiredWorkerCount() const;

	static FLayerInfo GetDefaultLayerInfo()
	{
		return { SpatialConstants::DefaultLayer, { AActor::StaticClass() }, USingleWorkerStrategy::StaticClass() };
	};

	UPROPERTY(EditAnywhere, Category = "Multiserver")
	TArray<FLayerInfo> WorkerLayers;

	UPROPERTY(EditAnywhere, Category = "Multiserver")
	TSubclassOf<UAbstractLockingPolicy> LockingPolicy;

private:
#if WITH_EDITOR
	void ValidateFirstLayerIsDefaultLayer();
	void ValidateNonEmptyWorkerLayers();
	void ValidateSomeLayerHasActorClass();
	void ValidateNoActorClassesDuplicatedAmongLayers();
	void ValidateAllLayersHaveUniqueNonemptyNames();
	void ValidateAllLayersHaveLoadBalancingStrategy();
	void ValidateLockingPolicyIsSet();
#endif
};

UCLASS(Blueprintable, HideDropdown)
class SPATIALGDK_API USpatialMultiserverSettings : public UAbstractSpatialMultiserverSettings
{
	GENERATED_BODY()

public:
	USpatialMultiserverSettings()
		: Super({ GetDefaultLayerInfo() }, UOwnershipLockingPolicy::StaticClass())
	{}
};
