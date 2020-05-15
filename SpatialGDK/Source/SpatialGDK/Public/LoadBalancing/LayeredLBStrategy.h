// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/AbstractLBStrategy.h"

#include "CoreMinimal.h"
#include "Math/Box2D.h"
#include "Math/Vector2D.h"

#include "LayeredLBStrategy.generated.h"

class SpatialVirtualWorkerTranslator;
class UAbstractLockingPolicy;

DECLARE_LOG_CATEGORY_EXTERN(LogLayeredLBStrategy, Log, All)

USTRUCT()
struct FLBLayerInfo
{
	GENERATED_BODY()

	FLBLayerInfo() : Name(NAME_None)
	{
	}

	UPROPERTY()
	FName Name;

	UPROPERTY(EditAnywhere, Category = "Load Balancing")
	TSubclassOf<UAbstractLBStrategy> LoadBalanceStrategy;

	UPROPERTY(EditAnywhere, Category = "Load Balancing")
	TSubclassOf<UAbstractLockingPolicy> LockingPolicy;
};

/**
 * A load balancing strategy that wraps multiple LBStrategies. The user can define "Layers" of work, which are
 * specified by sets of classes, and a LBStrategy for each Layer. This class will then allocate virtual workers
 * to each Layer/Strategy and keep track of which Actors belong in which layer and should be load balanced
 * by the corresponding Strategy.
 */
UCLASS()
class SPATIALGDK_API ULayeredLBStrategy : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	ULayeredLBStrategy();
	~ULayeredLBStrategy();

	/* UAbstractLBStrategy Interface */
	virtual void Init() override;

	virtual void SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId) override;

	virtual bool ShouldHaveAuthority(const AActor& Actor) const override;
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const override;

	virtual SpatialGDK::QueryConstraint GetWorkerInterestQueryConstraint() const override;

	virtual bool RequiresHandoverData() const override { return GetMinimumRequiredWorkers() > 1; }

	virtual FVector GetWorkerEntityPosition() const override;

	virtual uint32 GetMinimumRequiredWorkers() const override;
	virtual void SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId) override;
	/* End UAbstractLBStrategy Interface */

private:
	mutable TMap<TSoftClassPtr<AActor>, FName> ClassPathToLayer;

	TMap<VirtualWorkerId, FName> VirtualWorkerIdToLayerName;

	TMap<FName, UAbstractLBStrategy* > LayerNameToLBStrategy;

	// Returns the name of the first Layer that contains this, or a parent of this class,
	// or the default actor group, if no mapping is found.
	FName GetLayerNameForClass(TSubclassOf<AActor> Class) const;

	// Returns true if ActorA and ActorB are contained in Layers that are
	// on the same Server worker type.
	bool IsSameWorkerType(const AActor* ActorA, const AActor* ActorB) const;

	// Returns true if the current Worker Type owns this Actor Group.
	// Equivalent to World->GetNetMode() != NM_Client when Spatial Networking is disabled.
	bool IsLayerOwner(const FName& Layer) const;

	// Returns the name of the Layer this Actor belongs to.
	FName GetLayerNameForActor(const AActor& Actor) const;

	// Add a LBStrategy to our map and do bookkeeping around it.
	void AddStrategyForLayer(const FName& LayerName, UAbstractLBStrategy* LBStrategy);
};
