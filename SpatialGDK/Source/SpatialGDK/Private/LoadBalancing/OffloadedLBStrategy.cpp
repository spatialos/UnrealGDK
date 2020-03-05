// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/OffloadedLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SpatialActorUtils.h"

#include "Templates/Tuple.h"

DEFINE_LOG_CATEGORY(LogOffloadedLBStrategy);

UOffloadedLBStrategy::UOffloadedLBStrategy()
	: Super()
{
}

void UOffloadedLBStrategy::Init()
{
	Super::Init();

	// TODO(harkness): Find a way to query how many offloading types there are.
	VirtualWorkerIds.Add(1);
	VirtualWorkerIds.Add(2);

	if (const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>())
	{
		DefaultWorkerType = Settings->DefaultWorkerType.WorkerTypeName;
		if (Settings->bEnableOffloading)
		{
			for (const TPair<FName, FActorGroupInfo>& ActorGroup : Settings->ActorGroups)
			{
				ActorGroupToWorkerType.Add(ActorGroup.Key, ActorGroup.Value.OwningWorkerType.WorkerTypeName);

				for (const TSoftClassPtr<AActor>& ClassPtr : ActorGroup.Value.ActorClasses)
				{
					ClassPathToActorGroup.Add(ClassPtr, ActorGroup.Key);
				}
			}
		}
	}
}

TSet<VirtualWorkerId> UOffloadedLBStrategy::GetVirtualWorkerIds() const
{
	return TSet<VirtualWorkerId>(VirtualWorkerIds);
}

bool UOffloadedLBStrategy::ShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogOffloadedLBStrategy, Warning, TEXT("GridBasedLBStrategy not ready to relinquish authority for Actor %s."), *AActor::GetDebugName(&Actor));
		// TODO(harkness) Check if this is a valid return?
		return false;
	}

	return false;
}

VirtualWorkerId UOffloadedLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogOffloadedLBStrategy, Warning, TEXT("GridBasedLBStrategy not ready to decide on authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}

SpatialGDK::QueryConstraint UOffloadedLBStrategy::GetWorkerInterestQueryConstraint() const
{
	// For a grid-based strategy, the interest area is the cell that the worker is authoritative over plus some border region.
	check(IsReady());

	SpatialGDK::QueryConstraint Constraint;
	Constraint.ComponentConstraint = 0;
	return Constraint;
}

FVector UOffloadedLBStrategy::GetWorkerEntityPosition() const
{
	check(IsReady());
	return FVector{ 0.f, 0.f, 0.f };
}

FName UOffloadedLBStrategy::GetActorGroupForClass(const TSubclassOf<AActor> Class)
{
	if (Class == nullptr)
	{
		return NAME_None;
	}

	UClass* FoundClass = Class;
	TSoftClassPtr<AActor> ClassPtr = TSoftClassPtr<AActor>(FoundClass);

	while (FoundClass != nullptr && FoundClass->IsChildOf(AActor::StaticClass()))
	{
		if (const FName* ActorGroup = ClassPathToActorGroup.Find(ClassPtr))
		{
			FName ActorGroupHolder = *ActorGroup;
			if (FoundClass != Class)
			{
				ClassPathToActorGroup.Add(TSoftClassPtr<AActor>(Class), ActorGroupHolder);
			}
			return ActorGroupHolder;
		}

		FoundClass = FoundClass->GetSuperClass();
		ClassPtr = TSoftClassPtr<AActor>(FoundClass);
	}

	// No mapping found so set and return default actor group.
	ClassPathToActorGroup.Add(TSoftClassPtr<AActor>(Class), SpatialConstants::DefaultActorGroup);
	return SpatialConstants::DefaultActorGroup;
}

FName UOffloadedLBStrategy::GetWorkerTypeForClass(const TSubclassOf<AActor> Class)
{
	const FName ActorGroup = GetActorGroupForClass(Class);

	if (const FName* WorkerType = ActorGroupToWorkerType.Find(ActorGroup))
	{
		return *WorkerType;
	}

	return DefaultWorkerType;
}

FName UOffloadedLBStrategy::GetWorkerTypeForActorGroup(const FName& ActorGroup) const
{
	if (const FName* WorkerType = ActorGroupToWorkerType.Find(ActorGroup))
	{
		return *WorkerType;
	}

	return DefaultWorkerType;
}

bool UOffloadedLBStrategy::IsSameWorkerType(const AActor* ActorA, const AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return false;
	}

	const FName& WorkerTypeA = GetWorkerTypeForClass(ActorA->GetClass());
	const FName& WorkerTypeB = GetWorkerTypeForClass(ActorB->GetClass());

	return (WorkerTypeA == WorkerTypeB);
}
