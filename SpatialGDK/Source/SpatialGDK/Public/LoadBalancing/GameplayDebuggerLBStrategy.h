// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "LoadBalancing/AbstractLBStrategy.h"
#include "Utils/LayerInfo.h"

#include "GameplayDebuggerLBStrategy.generated.h"

class USpatialNetDriverGameplayDebuggerContext;
class UAbstractLBStrategy;

DECLARE_LOG_CATEGORY_EXTERN(LogGameplayDebuggerLBStrategy, Log, All)

/*
 * Load balancing strategy to manage gameplay debugger replicated actors.
 * This strategy wraps whatever LB strategy is already in place, and allows
 * said replicated actors to be authority assigned to any server.
 */
UCLASS(HideDropdown, NotBlueprintable)
class SPATIALGDK_API UGameplayDebuggerLBStrategy : public UAbstractLBStrategy
{
	GENERATED_BODY()

public:
	UGameplayDebuggerLBStrategy();

	void Init(USpatialNetDriverGameplayDebuggerContext& GameplayDebuggerCtx, UAbstractLBStrategy& InWrappedStrategy);

	/* UAbstractLBStrategy Interface */
	virtual void Init() override{};
	virtual FString ToString() const;
	virtual void SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId) override;
	virtual bool ShouldHaveAuthority(const AActor& Actor) const override;
	virtual VirtualWorkerId WhoShouldHaveAuthority(const AActor& Actor) const override;
	virtual SpatialGDK::FActorLoadBalancingGroupId GetActorGroupId(const AActor& Actor) const override;
	virtual SpatialGDK::QueryConstraint GetWorkerInterestQueryConstraint(const VirtualWorkerId VirtualWorker) const override;
	virtual FVector GetWorkerEntityPosition() const override;
	virtual uint32 GetMinimumRequiredWorkers() const override;
	virtual void SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId) override;
	virtual TSet<VirtualWorkerId> GetVirtualWorkerIds() const override;
	virtual UAbstractLBStrategy* GetLBStrategyForVisualRendering() const override;
	virtual bool RequiresHandoverData() const override;
	/* End UAbstractLBStrategy Interface */

	UAbstractLBStrategy* GetWrappedStrategy() const;

private:
	UPROPERTY()
	UAbstractLBStrategy* WrappedStrategy;

	USpatialNetDriverGameplayDebuggerContext* GameplayDebuggerCtx;
	TArray<VirtualWorkerId> VirtualWorkerIds;
};
