// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/AbstractLBStrategy.h"

#include "Utils/LayerInfo.h"

#include "Containers/Map.h"
#include "CoreMinimal.h"
#include "Math/Vector2D.h"

#include "LayeredLBStrategy.generated.h"

class SpatialVirtualWorkerTranslator;
class UAbstractLockingPolicy;
class UAbstractSpatialMultiWorkerSettings;

DECLARE_LOG_CATEGORY_EXTERN(LogLayeredLBStrategy, Log, All)

/**
 * A load balancing strategy that wraps multiple LBStrategies. The user can define "Layers" of work, which are
 * specified by sets of classes, and a LBStrategy for each Layer. This class will then allocate virtual workers
 * to each Layer/Strategy and keep track of which Actors belong in which layer and should be load balanced
 * by the corresponding Strategy.
 */
UCLASS(HideDropdown, NotBlueprintable)
class SPATIALGDK_API ULayeredLBStrategy : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	ULayeredLBStrategy();

	void SetLayers(const TArray<FLayerInfo>& WorkerLayers);

	/* UAbstractLBStrategy Interface */
	virtual void Init() override{};

	virtual void SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId) override;

	virtual TSet<VirtualWorkerId> GetVirtualWorkerIds() const override;

	virtual bool ShouldHaveAuthority(const AActor& Actor) const override;
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const override;

	virtual SpatialGDK::QueryConstraint GetWorkerInterestQueryConstraint() const override;

	virtual bool RequiresHandoverData() const override { return GetMinimumRequiredWorkers() > 1; }

	virtual FVector GetWorkerEntityPosition() const override;

	virtual uint32 GetMinimumRequiredWorkers() const override;
	virtual void SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId) override;
	/* End UAbstractLBStrategy Interface */

	// This is provided to support the offloading interface in SpatialStatics. It should be removed once users
	// switch to Load Balancing.
	bool CouldHaveAuthority(TSubclassOf<AActor> Class) const;

	// This returns the LBStrategy which should be rendered in the SpatialDebugger.
	// Currently, this is just the default strategy.
	UAbstractLBStrategy* GetLBStrategyForVisualRendering() const;

	FName GetLocalLayerName() const;

private:
	TArray<VirtualWorkerId> VirtualWorkerIds;

	mutable TMap<TSoftClassPtr<AActor>, FName> ClassPathToLayer;

	TMap<VirtualWorkerId, FName> VirtualWorkerIdToLayerName;

	UPROPERTY()
	TMap<FName, UAbstractLBStrategy*> LayerNameToLBStrategy;

	// Returns the name of the first Layer that contains this, or a parent of this class,
	// or the default actor group, if no mapping is found.
	FName GetLayerNameForClass(TSubclassOf<AActor> Class) const;

	// Returns true if ActorA and ActorB are contained in Layers that are
	// on the same Server worker type.
	bool IsSameWorkerType(const AActor* ActorA, const AActor* ActorB) const;

	// Returns the name of the Layer this Actor belongs to.
	FName GetLayerNameForActor(const AActor& Actor) const;

	// Add a LBStrategy to our map and do bookkeeping around it.
	void AddStrategyForLayer(const FName& LayerName, UAbstractLBStrategy* LBStrategy);
};
