// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/AbstractLBStrategy.h"

#include "Utils/LayerInfo.h"

#include "Containers/Map.h"
#include "CoreMinimal.h"
#include "Math/Vector2D.h"

#include "DebugLBStrategy.generated.h"

class UAbstractLockingPolicy;
class UAbstractSpatialMultiWorkerSettings;
class USpatialNetDriverDebugContext;

DECLARE_LOG_CATEGORY_EXTERN(LogDebugLBStrategy, Log, All)

/*
 * Debug load balancing strategy for SpatialFunctionalTest.
 * It is wrapping the load balancing strategy set on the NetDriver,
 * and inspecting debug tags before deferring to the wrapped strategy, effectively overriding it when needed.
 */
UCLASS(HideDropdown, NotBlueprintable)
class SPATIALGDK_API UDebugLBStrategy : public UAbstractLBStrategy
{
	GENERATED_BODY()
public:
	UDebugLBStrategy();
	void InitDebugStrategy(USpatialNetDriverDebugContext* DebugCtx, UAbstractLBStrategy* WrappedStrategy);

	/* UAbstractLBStrategy Interface */
	virtual void Init() override{};

	virtual void SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId) override;

	virtual TSet<VirtualWorkerId> GetVirtualWorkerIds() const override;

	virtual bool ShouldHaveAuthority(const AActor& Actor) const override;
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const override;

	virtual SpatialGDK::QueryConstraint GetWorkerInterestQueryConstraint(const VirtualWorkerId VirtualWorker) const override;

	virtual bool RequiresHandoverData() const override { return WrappedStrategy->RequiresHandoverData(); }

	virtual FVector GetWorkerEntityPosition() const override;

	virtual uint32 GetMinimumRequiredWorkers() const override;
	virtual void SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId) override;
	virtual UAbstractLBStrategy* GetLBStrategyForVisualRendering() const override;
	/* End UAbstractLBStrategy Interface */

	UAbstractLBStrategy* GetWrappedStrategy() const { return WrappedStrategy; }

private:
	UPROPERTY()
	UAbstractLBStrategy* WrappedStrategy = nullptr;

	USpatialNetDriverDebugContext* DebugCtx = nullptr;
};
