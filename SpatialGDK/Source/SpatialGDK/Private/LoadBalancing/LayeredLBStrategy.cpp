// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/LayeredLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
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

			const ASpatialWorldSettings* WorldSettings = GetWorld() ? Cast<ASpatialWorldSettings>(GetWorld()->GetWorldSettings()) : nullptr;

			if (WorldSettings == nullptr)
			{
				UE_LOG(LogLayeredLBStrategy, Error, TEXT("If EnableUnrealLoadBalancer is set, WorldSettings should inherit from SpatialWorldSettings to get the load balancing strategy."));
				return;
			}

			TMap<FName, FLBLayerInfo> WorkerLBLayers = WorldSettings->WorkerLBLayers;
			uint8 MinWorkersNeeded = 0;

			for (const TPair<FName, FLayerInfo>& Layer : Settings->WorkerLayers)
			{
				FName LayerKey = Layer.Key;

				// Look through the WorldSettings to find the LBStrategy type for this layer.
				if (!WorkerLBLayers.Contains(Layer.Key))
				{
					UE_LOG(LogLayeredLBStrategy, Error, TEXT("Layer %s does not have a defined LBStrategy in the WorldSettings. It will not be simulated."), *(LayerKey.ToString()));
					continue;
				}

				UAbstractLBStrategy* LBStrategy = NewObject<UAbstractLBStrategy>(this, WorkerLBLayers[Layer.Key].LoadBalanceStrategy);
				LayerNameToLBStrategy.Add(LayerKey, LBStrategy);
				LBStrategy->Init();
				LBStrategy->AddToRoot();
				MinWorkersNeeded += 0;  // TODO(harkness) query strategy for workers needed.

				for (const TSoftClassPtr<AActor>& ClassPtr : Layer.Value.ActorClasses)
				{
					ClassPathToLayer.Add(ClassPtr, LayerKey);
				}
			}

			// Finally, add the default layer.
			if (WorldSettings->DefaultLoadBalanceStrategy == nullptr)
			{
				UE_LOG(LogLayeredLBStrategy, Error, TEXT("If EnableUnrealLoadBalancer is set, there must be a LoadBalancing strategy set. Using a 1x1 grid."));
				UAbstractLBStrategy* DefaultLBStrategy = NewObject<UGridBasedLBStrategy>(this);
				DefaultLBStrategy->AddToRoot();
				LayerNameToLBStrategy.Emplace(SpatialConstants::DefaultLayer, DefaultLBStrategy);
				LayerNameToLBStrategy[SpatialConstants::DefaultLayer]->Init();
			}
			else
			{
				UAbstractLBStrategy* DefaultLBStrategy = NewObject<UAbstractLBStrategy>(this, WorldSettings->DefaultLoadBalanceStrategy);
				DefaultLBStrategy->AddToRoot();
				LayerNameToLBStrategy.Emplace(SpatialConstants::DefaultLayer, DefaultLBStrategy);
				LayerNameToLBStrategy[SpatialConstants::DefaultLayer]->Init();
				UE_LOG(LogLayeredLBStrategy, Log, TEXT("Added new strategy at location %lld"), DefaultLBStrategy);
			}
		}
	}
}

void ULayeredLBStrategy::SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId)
{
	LocalVirtualWorkerId = InLocalVirtualWorkerId;
	for (const auto& Elem : LayerNameToLBStrategy)
	{
		Elem.Value->SetLocalVirtualWorkerId(InLocalVirtualWorkerId - LayerNameToVirtualWorkerOffset[Elem.Key]);
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

	const FName& LayerKey = GetLayerForActor(Actor);
	if (!LayerNameToLBStrategy.Contains(LayerKey))
	{
		UE_LOG(LogLayeredLBStrategy, Warning, TEXT("LayeredLBStrategy doesn't have a LBStrategy for Actor %s which is in Layer %s."), *AActor::GetDebugName(&Actor), *LayerKey.ToString());
		return false;
	}

	if (VirtualWorkerIdToLayerName.Contains(LocalVirtualWorkerId) && VirtualWorkerIdToLayerName[LocalVirtualWorkerId] != GetLayerForActor(Actor))
	{
		return false;
	}

	return LayerNameToLBStrategy[LayerKey]->ShouldHaveAuthority(Actor);
}

VirtualWorkerId ULayeredLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogLayeredLBStrategy, Warning, TEXT("LayeredLBStrategy not ready to decide on authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	const FName& LayerKey = GetLayerForActor(Actor);
	if (!LayerNameToLBStrategy.Contains(LayerKey))
	{
		UE_LOG(LogLayeredLBStrategy, Warning, TEXT("LayeredLBStrategy doesn't have a LBStrategy for Actor %s which is in Layer %s."), *AActor::GetDebugName(&Actor), *LayerKey.ToString());
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	const VirtualWorkerId ReturnedWorkerId =  LayerNameToLBStrategy[LayerKey]->WhoShouldHaveAuthority(Actor);

	UE_LOG(LogLayeredLBStrategy, Log, TEXT("LayeredLBStrategy returning virtual worker id %d for Actor %s."), ReturnedWorkerId, *AActor::GetDebugName(&Actor));
	return ReturnedWorkerId + LayerNameToVirtualWorkerOffset[LayerKey];
}

SpatialGDK::QueryConstraint ULayeredLBStrategy::GetWorkerInterestQueryConstraint() const
{
	check(IsReady());
	if (!VirtualWorkerIdToLayerName.Contains(LocalVirtualWorkerId))
	{
		UE_LOG(LogLayeredLBStrategy, Warning, TEXT("LayeredLBStrategy doesn't have a LBStrategy for this worker"));
		SpatialGDK::QueryConstraint Constraint;
		Constraint.ComponentConstraint = 0;
		return Constraint;
	}
	else
	{
		return LayerNameToLBStrategy[VirtualWorkerIdToLayerName[LocalVirtualWorkerId]]->GetWorkerInterestQueryConstraint();
	}
}

FVector ULayeredLBStrategy::GetWorkerEntityPosition() const
{
	check(IsReady());
	if (!VirtualWorkerIdToLayerName.Contains(LocalVirtualWorkerId))
	{
		return FVector{ 0.f, 0.f, 0.f };
	}
	else
	{
		return LayerNameToLBStrategy[VirtualWorkerIdToLayerName[LocalVirtualWorkerId]]->GetWorkerEntityPosition();
	}
}

uint8 ULayeredLBStrategy::GetMinimumRequiredWorkers() const
{
	uint8 MinimumRequiredWorkers = 0;
	for (const auto& Elem : LayerNameToLBStrategy)
	{
		MinimumRequiredWorkers += Elem.Value->GetMinimumRequiredWorkers();
	}

	UE_LOG(LogLayeredLBStrategy, Log, TEXT("LayeredLBStrategy needs %d workers to support all layer strategies."), MinimumRequiredWorkers);
	return MinimumRequiredWorkers;
}

void ULayeredLBStrategy::SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId)
{
	VirtualWorkerId NextWorkerIdToAssign = FirstVirtualWorkerId;
	for (auto& Elem : LayerNameToLBStrategy)
	{
		VirtualWorkerId MinimumRequiredWorkers = Elem.Value->GetMinimumRequiredWorkers();
		LayerNameToVirtualWorkerOffset.Add(Elem.Key, NextWorkerIdToAssign - 1);

		if (NextWorkerIdToAssign + MinimumRequiredWorkers - 1 > LastVirtualWorkerId)
		{
			UE_LOG(LogLayeredLBStrategy, Error, TEXT("LayeredLBStrategy was not given enough VirtualWorkerIds to meet the demands of the layer strategies."));
			return;
		}

		Elem.Value->SetVirtualWorkerIds(1, MinimumRequiredWorkers);

		for (VirtualWorkerId id = 1; id <= MinimumRequiredWorkers; id++)
		{
			VirtualWorkerIdToLayerName.Add(id + LayerNameToVirtualWorkerOffset[Elem.Key], Elem.Key);
		}

		NextWorkerIdToAssign += MinimumRequiredWorkers;
	}
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

VirtualWorkerId ULayeredLBStrategy::GetVirtualWorkerIdForLayer(const FName& Layer) const
{
	if (const VirtualWorkerId* WorkerId = LayerNameToVirtualWorkerId.Find(Layer))
	{
		return *WorkerId;
	}

	// TODO(harkness): Need to clean this up.
	return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
}

bool ULayeredLBStrategy::IsSameWorkerType(const AActor* ActorA, const AActor* ActorB) const
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return false;
	}

	const FName& LayerA = GetLayerForClass(ActorA->GetClass());
	const FName& LayerB = GetLayerForClass(ActorB->GetClass());

	return (LayerA == LayerB);
}

// bool ULayeredLBStrategy::IsLayerOwnerForActor(const AActor* Actor) const
// {
// 	if (Actor == nullptr)
// 	{
// 		return false;
// 	}
// 
// 	return IsLayerOwnerForClass(Actor->GetClass());
// }

// Note: this is returning whether this is one of the workers which can simulate the layer. If there are
// multiple workers simulating the layer, there's no concept of owner.
bool ULayeredLBStrategy::IsLayerOwner(const FName Layer) const
{
	return *VirtualWorkerIdToLayerName.Find(LocalVirtualWorkerId) == Layer;
}

FName ULayeredLBStrategy::GetLayerForActor(const AActor& Actor) const
{
	return GetLayerForClass(Actor.GetClass());
}
