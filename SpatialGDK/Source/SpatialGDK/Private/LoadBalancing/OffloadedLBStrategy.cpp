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

	VirtualWorkerId CurrentVirtualWorkerId = SpatialConstants::INVALID_VIRTUAL_WORKER_ID + 1;

	if (const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>())
	{
		// TODO: can we just check that bEnableOffloading is true?
		DefaultWorkerType = Settings->DefaultWorkerType.WorkerTypeName;
		if (Settings->bEnableOffloading)
		{
			// First, add the default ActorGroup.
			VirtualWorkerIds.Add(CurrentVirtualWorkerId);
			ActorKeyToVirtualWorkerId.Add("Default", CurrentVirtualWorkerId++);

			for (const TPair<FName, FActorGroupInfo>& ActorGroup : Settings->ActorGroups)
			{
				FName WorkerTypeName = ActorGroup.Value.OwningWorkerType.WorkerTypeName;
				FName ActorGroupKey = ActorGroup.Key;
				if (!ActorKeyToVirtualWorkerId.Contains(ActorGroupKey))
				{
					UE_LOG(LogOffloadedLBStrategy, Log, TEXT("ActorGroup %s has been assigned to VirtualWorkerId %d"), *(ActorGroupKey.ToString()), CurrentVirtualWorkerId);
					VirtualWorkerIds.Add(CurrentVirtualWorkerId);
					ActorKeyToVirtualWorkerId.Add(ActorGroupKey, CurrentVirtualWorkerId++);
				}

				for (const TSoftClassPtr<AActor>& ClassPtr : ActorGroup.Value.ActorClasses)
				{
					ClassPathToActorGroup.Add(ClassPtr, ActorGroupKey);
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
		UE_LOG(LogOffloadedLBStrategy, Warning, TEXT("OffloadedLBStrategy not ready to relinquish authority for Actor %s."), *AActor::GetDebugName(&Actor));
		// TODO(harkness) Check if this is a valid return?
		return false;
	}

	return GetVirtualWorkerIdForClass(Actor.GetClass()) == LocalVirtualWorkerId;
}

VirtualWorkerId UOffloadedLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogOffloadedLBStrategy, Warning, TEXT("OffloadedLBStrategy not ready to decide on authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	UE_LOG(LogOffloadedLBStrategy, Log, TEXT("OffloadedLBStrategy returning virtual worker id %d for Actor %s."), GetVirtualWorkerIdForClass(Actor.GetClass()), *AActor::GetDebugName(&Actor));
	return GetVirtualWorkerIdForClass(Actor.GetClass());
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

FName UOffloadedLBStrategy::GetActorGroupForClass(const TSubclassOf<AActor> Class) const
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

VirtualWorkerId UOffloadedLBStrategy::GetVirtualWorkerIdForClass(const TSubclassOf<AActor> Class) const
{
	const FName ActorGroup = GetActorGroupForClass(Class);

	if (const VirtualWorkerId* WorkerId = ActorKeyToVirtualWorkerId.Find(ActorGroup))
	{
		return *WorkerId;
	}

	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}

VirtualWorkerId UOffloadedLBStrategy::GetVirtualWorkerIdForActorGroup(const FName& ActorGroup) const
{
	if (const VirtualWorkerId* WorkerId = ActorKeyToVirtualWorkerId.Find(ActorGroup))
	{
		return *WorkerId;
	}

	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}

bool UOffloadedLBStrategy::IsSameWorkerType(const AActor* ActorA, const AActor* ActorB) const
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return false;
	}

	const VirtualWorkerId VirtualWorkerIdA = GetVirtualWorkerIdForClass(ActorA->GetClass());
	const VirtualWorkerId VirtualWorkerIdB = GetVirtualWorkerIdForClass(ActorB->GetClass());

	return (VirtualWorkerIdA == VirtualWorkerIdB);
}

bool UOffloadedLBStrategy::IsActorGroupOwnerForActor(const AActor* Actor) const
{
	if (Actor == nullptr)
	{
		return false;
	}

	return IsActorGroupOwnerForClass(Actor->GetClass());
}

bool UOffloadedLBStrategy::IsActorGroupOwnerForClass(const TSubclassOf<AActor> ActorClass) const
{
	const VirtualWorkerId WorkerId = GetVirtualWorkerIdForClass(ActorClass);
	return WorkerId == LocalVirtualWorkerId;
}

bool UOffloadedLBStrategy::IsActorGroupOwner(const FName ActorGroup) const
{
	const VirtualWorkerId WorkerId = GetVirtualWorkerIdForActorGroup(ActorGroup);
	return WorkerId == LocalVirtualWorkerId;
}

FName UOffloadedLBStrategy::GetActorGroupForActor(const AActor* Actor) const
{
	return GetActorGroupForClass(Actor->GetClass());
}
