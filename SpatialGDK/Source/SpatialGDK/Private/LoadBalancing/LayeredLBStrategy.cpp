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
		if (Settings->bEnableMultiWorker)
		{
			const ASpatialWorldSettings* WorldSettings = GetWorld() ? Cast<ASpatialWorldSettings>(GetWorld()->GetWorldSettings()) : nullptr;

			if (WorldSettings == nullptr)
			{
				UE_LOG(LogLayeredLBStrategy, Error, TEXT("If EnableUnrealLoadBalancer is set, WorldSettings should inherit from SpatialWorldSettings to get the load balancing strategy."));
				return;
			}

			TMap<FName, FLBLayerInfo> WorkerLBLayers = WorldSettings->WorkerLBLayers;

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
				LayerNameToLBStrategy.Add(SpatialConstants::DefaultLayer, DefaultLBStrategy);
				LayerNameToLBStrategy[SpatialConstants::DefaultLayer]->Init();
			}
			else
			{
				UAbstractLBStrategy* DefaultLBStrategy = NewObject<UAbstractLBStrategy>(this, WorldSettings->DefaultLoadBalanceStrategy);
				DefaultLBStrategy->AddToRoot();
				LayerNameToLBStrategy.Add(SpatialConstants::DefaultLayer, DefaultLBStrategy);
				LayerNameToLBStrategy[SpatialConstants::DefaultLayer]->Init();
			}
		}
	}
}

void ULayeredLBStrategy::SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId)
{
	LocalVirtualWorkerId = InLocalVirtualWorkerId;
	for (const auto& Elem : LayerNameToLBStrategy)
	{
		Elem.Value->SetLocalVirtualWorkerId(InLocalVirtualWorkerId);
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
	UE_LOG(LogLayeredLBStrategy, Log, TEXT("LayeredLBStrategy found Layer %s for Actor %s."), *LayerKey.ToString(), *AActor::GetDebugName(&Actor));
	if (!LayerNameToLBStrategy.Contains(LayerKey))
	{
		UE_LOG(LogLayeredLBStrategy, Warning, TEXT("LayeredLBStrategy doesn't have a LBStrategy for Actor %s which is in Layer %s."), *AActor::GetDebugName(&Actor), *LayerKey.ToString());
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	const VirtualWorkerId ReturnedWorkerId = LayerNameToLBStrategy[LayerKey]->WhoShouldHaveAuthority(Actor);

	UE_LOG(LogLayeredLBStrategy, Log, TEXT("LayeredLBStrategy returning virtual worker id %d for Actor %s."), ReturnedWorkerId, *AActor::GetDebugName(&Actor));
	return ReturnedWorkerId;
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

		VirtualWorkerId LastVirtualWorkerIdToAssign = NextWorkerIdToAssign + MinimumRequiredWorkers - 1;
		if (LastVirtualWorkerIdToAssign > LastVirtualWorkerId)
		{
			UE_LOG(LogLayeredLBStrategy, Error, TEXT("LayeredLBStrategy was not given enough VirtualWorkerIds to meet the demands of the layer strategies."));
			return;
		}
		Elem.Value->SetVirtualWorkerIds(NextWorkerIdToAssign, LastVirtualWorkerIdToAssign);

		for (VirtualWorkerId id = NextWorkerIdToAssign; id <= LastVirtualWorkerIdToAssign; id++)
		{
			VirtualWorkerIdToLayerName.Add(id, Elem.Key);
		}

		NextWorkerIdToAssign = LastVirtualWorkerIdToAssign + 1;
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
