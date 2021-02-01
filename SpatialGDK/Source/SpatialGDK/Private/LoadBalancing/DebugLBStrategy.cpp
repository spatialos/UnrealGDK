// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/DebugLBStrategy.h"

#include "EngineClasses/SpatialNetDriverDebugContext.h"
#include "EngineClasses/SpatialWorldSettings.h"

DEFINE_LOG_CATEGORY(LogDebugLBStrategy);

UDebugLBStrategy::UDebugLBStrategy() = default;

void UDebugLBStrategy::InitDebugStrategy(USpatialNetDriverDebugContext* InDebugCtx, UAbstractLBStrategy* InWrappedStrategy)
{
	DebugCtx = InDebugCtx;
	WrappedStrategy = InWrappedStrategy;
	LocalVirtualWorkerId = InWrappedStrategy->GetLocalVirtualWorkerId();
}

void UDebugLBStrategy::SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId)
{
	check(WrappedStrategy);
	WrappedStrategy->SetLocalVirtualWorkerId(InLocalVirtualWorkerId);
	LocalVirtualWorkerId = WrappedStrategy->GetLocalVirtualWorkerId();
}

TSet<VirtualWorkerId> UDebugLBStrategy::GetVirtualWorkerIds() const
{
	check(WrappedStrategy);
	return WrappedStrategy->GetVirtualWorkerIds();
}

bool UDebugLBStrategy::ShouldHaveAuthority(const AActor& Actor) const
{
	check(WrappedStrategy);

	TOptional<VirtualWorkerId> WorkerId = DebugCtx->GetActorHierarchyExplicitDelegation(&Actor);
	if (WorkerId)
	{
		return WorkerId.GetValue() == GetLocalVirtualWorkerId();
	}

	return WrappedStrategy->ShouldHaveAuthority(Actor);
}

VirtualWorkerId UDebugLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	check(WrappedStrategy);

	TOptional<VirtualWorkerId> WorkerId = DebugCtx->GetActorHierarchyExplicitDelegation(&Actor);
	if (WorkerId)
	{
		return WorkerId.GetValue();
	}

	return WrappedStrategy->WhoShouldHaveAuthority(Actor);
}

SpatialGDK::QueryConstraint UDebugLBStrategy::GetWorkerInterestQueryConstraint(const VirtualWorkerId VirtualWorker) const
{
	check(WrappedStrategy);

	SpatialGDK::QueryConstraint DefaultConstraint = WrappedStrategy->GetWorkerInterestQueryConstraint(VirtualWorker);
	SpatialGDK::QueryConstraint AdditionalConstraint = DebugCtx->ComputeAdditionalEntityQueryConstraint();
	DebugCtx->ClearNeedEntityInterestUpdate();

	if (AdditionalConstraint.IsValid())
	{
		SpatialGDK::QueryConstraint WorkerConstraint;
		WorkerConstraint.OrConstraint.Add(DefaultConstraint);
		WorkerConstraint.OrConstraint.Add(AdditionalConstraint);

		return WorkerConstraint;
	}

	return DefaultConstraint;
}

FVector UDebugLBStrategy::GetWorkerEntityPosition() const
{
	check(WrappedStrategy);
	return WrappedStrategy->GetWorkerEntityPosition();
}

uint32 UDebugLBStrategy::GetMinimumRequiredWorkers() const
{
	check(WrappedStrategy);
	return WrappedStrategy->GetMinimumRequiredWorkers();
}

void UDebugLBStrategy::SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId)
{
	check(WrappedStrategy);
	WrappedStrategy->SetVirtualWorkerIds(FirstVirtualWorkerId, LastVirtualWorkerId);
}

UAbstractLBStrategy* UDebugLBStrategy::GetLBStrategyForVisualRendering() const
{
	check(WrappedStrategy);
	return WrappedStrategy->GetLBStrategyForVisualRendering();
}
