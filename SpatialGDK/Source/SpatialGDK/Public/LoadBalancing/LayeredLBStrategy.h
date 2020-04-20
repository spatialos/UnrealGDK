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

	UPROPERTY()
	FName Name;

	UPROPERTY(EditAnywhere, Category = "Load Balancing")
	TSubclassOf<UAbstractLBStrategy> LoadBalanceStrategy;

	UPROPERTY(EditAnywhere, Category = "Load Balancing")
	TSubclassOf<UAbstractLockingPolicy> LockingPolicy;


	FLBLayerInfo() : Name(NAME_None)
	{
	}
};

/**
 * A load balancing strategy that ...
 */
UCLASS(Blueprintable)
class SPATIALGDK_API ULayeredLBStrategy : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	ULayeredLBStrategy();

	/* UAbstractLBStrategy Interface */
	virtual void Init() override;

	void SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId);

	virtual TSet<VirtualWorkerId> GetVirtualWorkerIds() const override;

	virtual bool ShouldHaveAuthority(const AActor& Actor) const override;
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const override;

	virtual SpatialGDK::QueryConstraint GetWorkerInterestQueryConstraint() const override;

	virtual FVector GetWorkerEntityPosition() const override;

	virtual uint8 GetMinimumRequiredWorkers() const override;
	virtual void SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId)  override;
	/* End UAbstractLBStrategy Interface */

protected:

private:

	TArray<VirtualWorkerId> VirtualWorkerIds;

	mutable TMap<TSoftClassPtr<AActor>, FName> ClassPathToLayer;

	TMap<FName, VirtualWorkerId> LayerNameToVirtualWorkerId;
	TMap<VirtualWorkerId, FName> VirtualWorkerIdToLayerName;

	// UPROPERTY()
	TMap<FName, UAbstractLBStrategy* > LayerNameToLBStrategy;

	TMap<FName, uint32>  LayerNameToVirtualWorkerOffset;

	FName DefaultWorkerType;

	// Returns the first Layer that contains this, or a parent of this class,
	// or the default actor group, if no mapping is found.
	FName GetLayerForClass(TSubclassOf<AActor> Class) const;

	// Returns true if ActorA and ActorB are contained in Layers that are
	// on the same Server worker type.
	bool IsSameWorkerType(const AActor* ActorA, const AActor* ActorB) const;

	/**
	 * Returns true if the current Worker Type owns this Actor Group.
	 * Equivalent to World->GetNetMode() != NM_Client when Spatial Networking is disabled.
	 */
	bool IsLayerOwner(const FName Layer) const;

	/**
	 * Returns the Layer this Actor belongs to.
	 */
	FName GetLayerForActor(const AActor& Actor) const;
};
