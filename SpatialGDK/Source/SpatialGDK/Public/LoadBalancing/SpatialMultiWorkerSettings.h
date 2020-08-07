// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/LayeredLBStrategy.h"
#include "LoadBalancing/OwnershipLockingPolicy.h"
#include "SpatialConstants.h"
#include "Utils/LayerInfo.h"

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "SpatialMultiWorkerSettings.generated.h"

class UAbstractLockingPolicy;

UCLASS(NotBlueprintable)
class SPATIALGDK_API UAbstractSpatialMultiWorkerSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UAbstractSpatialMultiWorkerSettings() {}

protected:
	UAbstractSpatialMultiWorkerSettings(TArray<FLayerInfo> InWorkerLayers, TSubclassOf<UAbstractLockingPolicy> InLockingPolicy)
		: WorkerLayers(InWorkerLayers)
		, LockingPolicy(InLockingPolicy)
	{
	}

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	uint32 GetMinimumRequiredWorkerCount() const;

	static FLayerInfo GetDefaultLayerInfo()
	{
		return { SpatialConstants::DefaultLayer, { AActor::StaticClass() }, USingleWorkerStrategy::StaticClass() };
	};

	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
	TArray<FLayerInfo> WorkerLayers;

	UPROPERTY(EditAnywhere, Category = "Multi-Worker")
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
class SPATIALGDK_API USpatialMultiWorkerSettings : public UAbstractSpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	USpatialMultiWorkerSettings()
		: Super({ UAbstractSpatialMultiWorkerSettings::GetDefaultLayerInfo() }, UOwnershipLockingPolicy::StaticClass())
	{
	}
};
