// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/LayeredLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "Utils/SpatialActorUtils.h"

#include "Templates/Tuple.h"

DEFINE_LOG_CATEGORY(LogLayeredLBStrategy);

ULayeredLBStrategy::ULayeredLBStrategy()
	: Super()
{
}

void ULayeredLBStrategy::Init()
{
	Super::Init();

	VirtualWorkerId CurrentVirtualWorkerId = SpatialConstants::INVALID_VIRTUAL_WORKER_ID + 1;

	if (const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>())
	{
		// TODO: can we just check that bEnableOffloading is true?
		DefaultWorkerType = Settings->DefaultWorkerType.WorkerTypeName;
		if (Settings->bEnableMultiWorker)  // TODO(harkness) need to check something else here?
		{
			// First, add the default Layer.
			VirtualWorkerIds.Add(CurrentVirtualWorkerId);
			ActorKeyToVirtualWorkerId.Add("Default", CurrentVirtualWorkerId++);

			for (const TPair<FName, FLayerInfo>& Layer : Settings->WorkerLayers)
			{
				FName WorkerTypeName = Layer.Value.OwningWorkerType.WorkerTypeName;
				FName LayerKey = Layer.Key;
				if (!ActorKeyToVirtualWorkerId.Contains(LayerKey))
				{
					UE_LOG(LogLayeredLBStrategy, Log, TEXT("Layer %s has been assigned to VirtualWorkerId %d"), *(LayerKey.ToString()), CurrentVirtualWorkerId);
					VirtualWorkerIds.Add(CurrentVirtualWorkerId);
					ActorKeyToVirtualWorkerId.Add(LayerKey, CurrentVirtualWorkerId++);
				}

				for (const TSoftClassPtr<AActor>& ClassPtr : Layer.Value.ActorClasses)
				{
					ClassPathToLayer.Add(ClassPtr, LayerKey);
				}
			}
		}
	}
}

TSet<VirtualWorkerId> ULayeredLBStrategy::GetVirtualWorkerIds() const
{
	return TSet<VirtualWorkerId>(VirtualWorkerIds);
}

bool ULayeredLBStrategy::ShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogLayeredLBStrategy, Warning, TEXT("LayeredLBStrategy not ready to relinquish authority for Actor %s."), *AActor::GetDebugName(&Actor));
		// TODO(harkness) Check if this is a valid return?
		return false;
	}

	return GetVirtualWorkerIdForClass(Actor.GetClass()) == LocalVirtualWorkerId;
}

VirtualWorkerId ULayeredLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogLayeredLBStrategy, Warning, TEXT("LayeredLBStrategy not ready to decide on authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	UE_LOG(LogLayeredLBStrategy, Log, TEXT("LayeredLBStrategy returning virtual worker id %d for Actor %s."), GetVirtualWorkerIdForClass(Actor.GetClass()), *AActor::GetDebugName(&Actor));
	return GetVirtualWorkerIdForClass(Actor.GetClass());
}

SpatialGDK::QueryConstraint ULayeredLBStrategy::GetWorkerInterestQueryConstraint() const
{
	// For a grid-based strategy, the interest area is the cell that the worker is authoritative over plus some border region.
	check(IsReady());

	SpatialGDK::QueryConstraint Constraint;
	Constraint.ComponentConstraint = 0;
	return Constraint;
}

FVector ULayeredLBStrategy::GetWorkerEntityPosition() const
{
	check(IsReady());
	return FVector{ 0.f, 0.f, 0.f };
}

FName ULayeredLBStrategy::GetLayerForClass(const TSubclassOf<AActor> Class) const
{
	if (Class == nullptr)
	{
		return NAME_None;
	}

	UClass* FoundClass = Class;
	TSoftClassPtr<AActor> ClassPtr = TSoftClassPtr<AActor>(FoundClass);

	while (FoundClass != nullptr && FoundClass->IsChildOf(AActor::StaticClass()))
	{
		if (const FName* Layer = ClassPathToLayer.Find(ClassPtr))
		{
			FName LayerHolder = *Layer;
			if (FoundClass != Class)
			{
				ClassPathToLayer.Add(TSoftClassPtr<AActor>(Class), LayerHolder);
			}
			return LayerHolder;
		}

		FoundClass = FoundClass->GetSuperClass();
		ClassPtr = TSoftClassPtr<AActor>(FoundClass);
	}

	// No mapping found so set and return default actor group.
	ClassPathToLayer.Add(TSoftClassPtr<AActor>(Class), SpatialConstants::DefaultLayer);
	return SpatialConstants::DefaultLayer;
}

VirtualWorkerId ULayeredLBStrategy::GetVirtualWorkerIdForClass(const TSubclassOf<AActor> Class) const
{
	const FName Layer = GetLayerForClass(Class);

	if (const VirtualWorkerId* WorkerId = ActorKeyToVirtualWorkerId.Find(Layer))
	{
		return *WorkerId;
	}

	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}

VirtualWorkerId ULayeredLBStrategy::GetVirtualWorkerIdForLayer(const FName& Layer) const
{
	if (const VirtualWorkerId* WorkerId = ActorKeyToVirtualWorkerId.Find(Layer))
	{
		return *WorkerId;
	}

	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}

bool ULayeredLBStrategy::IsSameWorkerType(const AActor* ActorA, const AActor* ActorB) const
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return false;
	}

	const VirtualWorkerId VirtualWorkerIdA = GetVirtualWorkerIdForClass(ActorA->GetClass());
	const VirtualWorkerId VirtualWorkerIdB = GetVirtualWorkerIdForClass(ActorB->GetClass());

	return (VirtualWorkerIdA == VirtualWorkerIdB);
}

bool ULayeredLBStrategy::IsLayerOwnerForActor(const AActor* Actor) const
{
	if (Actor == nullptr)
	{
		return false;
	}

	return IsLayerOwnerForClass(Actor->GetClass());
}

bool ULayeredLBStrategy::IsLayerOwnerForClass(const TSubclassOf<AActor> ActorClass) const
{
	const VirtualWorkerId WorkerId = GetVirtualWorkerIdForClass(ActorClass);
	return WorkerId == LocalVirtualWorkerId;
}

bool ULayeredLBStrategy::IsLayerOwner(const FName Layer) const
{
	const VirtualWorkerId WorkerId = GetVirtualWorkerIdForLayer(Layer);
	return WorkerId == LocalVirtualWorkerId;
}

FName ULayeredLBStrategy::GetLayerForActor(const AActor* Actor) const
{
	return GetLayerForClass(Actor->GetClass());
}
