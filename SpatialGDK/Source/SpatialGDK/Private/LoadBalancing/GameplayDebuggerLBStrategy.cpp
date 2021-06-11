// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/GameplayDebuggerLBStrategy.h"

#include "EngineClasses/SpatialNetDriverGameplayDebuggerContext.h"
#include "EngineClasses/SpatialWorldSettings.h"

DEFINE_LOG_CATEGORY(LogGameplayDebuggerLBStrategy);

UGameplayDebuggerLBStrategy::UGameplayDebuggerLBStrategy()
	: WrappedStrategy(nullptr)
	, GameplayDebuggerCtx(nullptr)
	, VirtualWorkerIds()
{
}

void UGameplayDebuggerLBStrategy::Init(USpatialNetDriverGameplayDebuggerContext& InGameplayDebuggerCtx,
	UAbstractLBStrategy& InWrappedStrategy)
{
	GameplayDebuggerCtx = &InGameplayDebuggerCtx;
	WrappedStrategy = &InWrappedStrategy;
	LocalVirtualWorkerId = InWrappedStrategy.GetLocalVirtualWorkerId();
}

FString UGameplayDebuggerLBStrategy::ToString() const
{
	return TEXT("GameplayDebugger");
}

void UGameplayDebuggerLBStrategy::SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId)
{
	check(WrappedStrategy);
	WrappedStrategy->SetLocalVirtualWorkerId(InLocalVirtualWorkerId);
	LocalVirtualWorkerId = WrappedStrategy->GetLocalVirtualWorkerId();
}

bool UGameplayDebuggerLBStrategy::ShouldHaveAuthority(const AActor& Actor) const
{
	check(WrappedStrategy);

	TOptional<VirtualWorkerId> WorkerId = GameplayDebuggerCtx->GetActorDelegatedWorkerId(Actor);
	if (WorkerId)
	{
		return WorkerId.GetValue() == GetLocalVirtualWorkerId();
	}

	return WrappedStrategy->ShouldHaveAuthority(Actor);
}

VirtualWorkerId UGameplayDebuggerLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	check(WrappedStrategy);

	TOptional<VirtualWorkerId> WorkerId = GameplayDebuggerCtx->GetActorDelegatedWorkerId(Actor);
	if (WorkerId)
	{
		return WorkerId.GetValue();
	}

	return WrappedStrategy->WhoShouldHaveAuthority(Actor);
}

SpatialGDK::FActorLoadBalancingGroupId UGameplayDebuggerLBStrategy::GetActorGroupId(const AActor& Actor) const
{
	check(WrappedStrategy);
	return WrappedStrategy->GetActorGroupId(Actor);
}

SpatialGDK::QueryConstraint UGameplayDebuggerLBStrategy::GetWorkerInterestQueryConstraint(const VirtualWorkerId VirtualWorker) const
{
	check(WrappedStrategy);

	SpatialGDK::QueryConstraint DefaultConstraint = WrappedStrategy->GetWorkerInterestQueryConstraint(VirtualWorker);
	SpatialGDK::QueryConstraint AdditionalConstraint = GameplayDebuggerCtx->ComputeAdditionalEntityQueryConstraint();
	GameplayDebuggerCtx->ClearNeedEntityInterestUpdate();

	if (AdditionalConstraint.IsValid())
	{
		SpatialGDK::QueryConstraint WorkerConstraint;
		WorkerConstraint.OrConstraint.Add(DefaultConstraint);
		WorkerConstraint.OrConstraint.Add(AdditionalConstraint);

		return WorkerConstraint;
	}

	return DefaultConstraint;
}

FVector UGameplayDebuggerLBStrategy::GetWorkerEntityPosition() const
{
	check(WrappedStrategy);
	return WrappedStrategy->GetWorkerEntityPosition();
}

uint32 UGameplayDebuggerLBStrategy::GetMinimumRequiredWorkers() const
{
	check(WrappedStrategy);
	return WrappedStrategy->GetMinimumRequiredWorkers();
}

void UGameplayDebuggerLBStrategy::SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId)
{
	check(WrappedStrategy);
	WrappedStrategy->SetVirtualWorkerIds(FirstVirtualWorkerId, LastVirtualWorkerId);
}

TSet<VirtualWorkerId> UGameplayDebuggerLBStrategy::GetVirtualWorkerIds() const
{
	check(WrappedStrategy);
	return WrappedStrategy->GetVirtualWorkerIds();
}

UAbstractLBStrategy* UGameplayDebuggerLBStrategy::GetLBStrategyForVisualRendering() const
{
	check(WrappedStrategy);
	return WrappedStrategy->GetLBStrategyForVisualRendering();
}

UAbstractLBStrategy* UGameplayDebuggerLBStrategy::GetWrappedStrategy() const
{
	return WrappedStrategy;
}


