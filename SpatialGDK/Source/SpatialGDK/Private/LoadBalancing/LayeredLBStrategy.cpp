// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/LayeredLBStrategy.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "Utils/LayerInfo.h"
#include "Utils/SpatialActorUtils.h"

#include "Templates/Tuple.h"
#include "Containers/Map.h"

DEFINE_LOG_CATEGORY(LogLayeredLBStrategy);

ULayeredLBStrategy::ULayeredLBStrategy()
	: Super()
{
}

void ULayeredLBStrategy::Init(const UAbstractSpatialMultiWorkerSettings* MultiWorkerSettings)
{
	Super::Init(MultiWorkerSettings);

	check(MultiWorkerSettings->WorkerLayers.Num() != 0);

	// For each Layer, add a LB Strategy for that layer.
	for (const FLayerInfo& LayerInfo : MultiWorkerSettings->WorkerLayers)
	{
		UAbstractLBStrategy* LBStrategy;
		if (LayerInfo.LoadBalanceStrategy == nullptr)
		{
			UE_LOG(LogLayeredLBStrategy, Error, TEXT("WorkerLayer %s does not specify a loadbalancing strategy "
				"(or it cannot be resolved). Using a 1x1 grid."), *LayerInfo.Name.ToString());
			LBStrategy = NewObject<UGridBasedLBStrategy>(this);
		}
		else
		{
			LBStrategy = NewObject<UAbstractLBStrategy>(this, LayerInfo.LoadBalanceStrategy);
		}

		AddStrategyForLayer(LayerInfo.Name, LBStrategy);

		UE_LOG(LogLayeredLBStrategy, Log, TEXT("Creating LBStrategy for Layer %s."), *LayerInfo.Name.ToString());
		for (const TSoftClassPtr<AActor>& ClassPtr : LayerInfo.ActorClasses)
		{
			UE_LOG(LogLayeredLBStrategy, Log, TEXT(" - Adding class %s."), *ClassPtr.GetAssetName());
			ClassPathToLayer.Add(ClassPtr, LayerInfo.Name);
		}
	}

	//// Finally, add the default layer.
	//UE_LOG(LogLayeredLBStrategy, Log, TEXT("Creating LBStrategy for the Default Layer."));
	//TSubclassOf<UAbstractLBStrategy> DefaultLayerLoadBalanceStrategy = MultiWorkerSettings->DefaultLayerLoadBalanceStrategy;
	//if (*DefaultLayerLoadBalanceStrategy == nullptr)
	//{
	//	UE_LOG(LogLayeredLBStrategy, Error, TEXT("If EnableMultiWorker is set, there must be a LoadBalancing strategy set. Using a 1x1 grid."));
	//	UAbstractLBStrategy* DefaultLBStrategy = NewObject<UGridBasedLBStrategy>(this);
	//	AddStrategyForLayer(SpatialConstants::DefaultLayer, DefaultLBStrategy);
	//}
	//else
	//{
	//	UAbstractLBStrategy* DefaultLBStrategy = NewObject<UAbstractLBStrategy>(this, DefaultLayerLoadBalanceStrategy);
	//	AddStrategyForLayer(SpatialConstants::DefaultLayer, DefaultLBStrategy);

	//	// Any class not specified on one of the other layers will be on the default layer. However, some games may have a class hierarchy with
	//	// some parts of the hierarchy on different layers. This provides a way to specify that.
	//	for (const TSoftClassPtr<AActor>& ClassPtr : WorldSettings->ExplicitDefaultActorClasses)
	//	{
	//		UE_LOG(LogLayeredLBStrategy, Log, TEXT(" - Adding class to default layer %s."), *ClassPtr.GetAssetName());
	//		ClassPathToLayer.Add(ClassPtr, SpatialConstants::DefaultLayer);
	//	}
	//}
}

void ULayeredLBStrategy::SetLocalVirtualWorkerId(VirtualWorkerId InLocalVirtualWorkerId)
{
	if (LocalVirtualWorkerId != SpatialConstants::INVALID_VIRTUAL_WORKER_ID)
	{
		UE_LOG(LogLayeredLBStrategy, Error,
			TEXT("The Local Virtual Worker Id cannot be set twice. Current value: %d Requested new value: %d"),
			LocalVirtualWorkerId, InLocalVirtualWorkerId);
		return;
	}

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

	const AActor* RootOwner = &Actor;
	while (RootOwner->GetOwner() != nullptr && RootOwner->GetOwner()->GetIsReplicated())
	{
		RootOwner = RootOwner->GetOwner();
	}

	const FName& LayerName = GetLayerNameForActor(*RootOwner);
	if (!LayerNameToLBStrategy.Contains(LayerName))
	{
		UE_LOG(LogLayeredLBStrategy, Error, TEXT("LayeredLBStrategy doesn't have a LBStrategy for Actor %s which is in Layer %s."), *AActor::GetDebugName(RootOwner), *LayerName.ToString());
		return false;
	}

	// If this worker is not responsible for the Actor's layer, just return false.
	if (VirtualWorkerIdToLayerName.Contains(LocalVirtualWorkerId) && VirtualWorkerIdToLayerName[LocalVirtualWorkerId] != LayerName)
	{
		return false;
	}

	return LayerNameToLBStrategy[LayerName]->ShouldHaveAuthority(Actor);
}

VirtualWorkerId ULayeredLBStrategy::WhoShouldHaveAuthority(const AActor& Actor) const
{
	if (!IsReady())
	{
		UE_LOG(LogLayeredLBStrategy, Warning, TEXT("LayeredLBStrategy not ready to decide on authority for Actor %s."), *AActor::GetDebugName(&Actor));
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	const AActor* RootOwner = &Actor;
	while (RootOwner->GetOwner() != nullptr && RootOwner->GetOwner()->GetIsReplicated())
	{
		RootOwner = RootOwner->GetOwner();
	}

	const FName& LayerName = GetLayerNameForActor(*RootOwner);
	if (!LayerNameToLBStrategy.Contains(LayerName))
	{
		UE_LOG(LogLayeredLBStrategy, Error, TEXT("LayeredLBStrategy doesn't have a LBStrategy for Actor %s which is in Layer %s."), *AActor::GetDebugName(RootOwner), *LayerName.ToString());
		return SpatialConstants::INVALID_VIRTUAL_WORKER_ID;
	}

	const VirtualWorkerId ReturnedWorkerId = LayerNameToLBStrategy[LayerName]->WhoShouldHaveAuthority(*RootOwner);

	UE_LOG(LogLayeredLBStrategy, Log, TEXT("LayeredLBStrategy returning virtual worker id %d for Actor %s."), ReturnedWorkerId, *AActor::GetDebugName(RootOwner));
	return ReturnedWorkerId;
}

SpatialGDK::QueryConstraint ULayeredLBStrategy::GetWorkerInterestQueryConstraint() const
{
	check(IsReady());
	if (!VirtualWorkerIdToLayerName.Contains(LocalVirtualWorkerId))
	{
		UE_LOG(LogLayeredLBStrategy, Error, TEXT("LayeredLBStrategy doesn't have a LBStrategy for worker %d."), LocalVirtualWorkerId);
		SpatialGDK::QueryConstraint Constraint;
		Constraint.ComponentConstraint = 0;
		return Constraint;
	}
	else
	{
		const FName& LayerName = VirtualWorkerIdToLayerName[LocalVirtualWorkerId];
		check(LayerNameToLBStrategy.Contains(LayerName));
		return LayerNameToLBStrategy[LayerName]->GetWorkerInterestQueryConstraint();
	}
}

FVector ULayeredLBStrategy::GetWorkerEntityPosition() const
{
	check(IsReady());
	if (!VirtualWorkerIdToLayerName.Contains(LocalVirtualWorkerId))
	{
		UE_LOG(LogLayeredLBStrategy, Error, TEXT("LayeredLBStrategy doesn't have a LBStrategy for worker %d."), LocalVirtualWorkerId);
		return FVector{ 0.f, 0.f, 0.f };
	}
	else
	{
		const FName& LayerName = VirtualWorkerIdToLayerName[LocalVirtualWorkerId];
		check(LayerNameToLBStrategy.Contains(LayerName));
		return LayerNameToLBStrategy[LayerName]->GetWorkerEntityPosition();
	}
}

uint32 ULayeredLBStrategy::GetMinimumRequiredWorkers() const
{
	// The MinimumRequiredWorkers for this strategy is a sum of the required workers for each of the wrapped strategies.
	uint32 MinimumRequiredWorkers = 0;
	for (const auto& Elem : LayerNameToLBStrategy)
	{
		MinimumRequiredWorkers += Elem.Value->GetMinimumRequiredWorkers();
	}

	UE_LOG(LogLayeredLBStrategy, Verbose, TEXT("LayeredLBStrategy needs %d workers to support all layer strategies."), MinimumRequiredWorkers);
	return MinimumRequiredWorkers;
}

void ULayeredLBStrategy::SetVirtualWorkerIds(const VirtualWorkerId& FirstVirtualWorkerId, const VirtualWorkerId& LastVirtualWorkerId)
{
	// If the LayeredLBStrategy wraps { SingletonStrategy, 2x2 grid, Singleton } and is given IDs 1 through 6 it will assign:
	// Singleton : 1
	// Grid : 2 - 5
	// Singleton: 6
	VirtualWorkerId NextWorkerIdToAssign = FirstVirtualWorkerId;
	for (const auto& Elem : LayerNameToLBStrategy)
	{
		UAbstractLBStrategy* LBStrategy = Elem.Value;
		VirtualWorkerId MinimumRequiredWorkers = LBStrategy->GetMinimumRequiredWorkers();

		VirtualWorkerId LastVirtualWorkerIdToAssign = NextWorkerIdToAssign + MinimumRequiredWorkers - 1;
		if (LastVirtualWorkerIdToAssign > LastVirtualWorkerId)
		{
			UE_LOG(LogLayeredLBStrategy, Error, TEXT("LayeredLBStrategy was not given enough VirtualWorkerIds to meet the demands of the layer strategies."));
			return;
		}
		UE_LOG(LogLayeredLBStrategy, Log, TEXT("LayeredLBStrategy assigning VirtualWorkerIds %d to %d to Layer %s"), NextWorkerIdToAssign, LastVirtualWorkerIdToAssign, *Elem.Key.ToString());
		LBStrategy->SetVirtualWorkerIds(NextWorkerIdToAssign, LastVirtualWorkerIdToAssign);

		for (VirtualWorkerId id = NextWorkerIdToAssign; id <= LastVirtualWorkerIdToAssign; id++)
		{
			VirtualWorkerIdToLayerName.Add(id, Elem.Key);
		}

		NextWorkerIdToAssign += MinimumRequiredWorkers;
	}

	// Keep a copy of the VirtualWorkerIds. This is temporary and will be removed in the next PR.
	for (VirtualWorkerId CurrentVirtualWorkerId = FirstVirtualWorkerId; CurrentVirtualWorkerId <= LastVirtualWorkerId; CurrentVirtualWorkerId++)
	{
		VirtualWorkerIds.Add(CurrentVirtualWorkerId);
	}
}

// DEPRECATED
// This is only included because Scavengers uses the function in SpatialStatics that calls this.
// Once they are pick up this code, they should be able to switch to another method and we can remove this.
bool ULayeredLBStrategy::CouldHaveAuthority(const TSubclassOf<AActor> Class) const
{
	check(IsReady());
	return *VirtualWorkerIdToLayerName.Find(LocalVirtualWorkerId) == GetLayerNameForClass(Class);
}

UAbstractLBStrategy* ULayeredLBStrategy::GetLBStrategyForVisualRendering() const
{
	// The default strategy is guaranteed to exist as long as the strategy is ready.
	check(IsReady());

	if (LayerNameToLBStrategy.Contains(SpatialConstants::DefaultLayer))
	{ 
		return LayerNameToLBStrategy[SpatialConstants::DefaultLayer];
	}

	UE_LOG(LogLayeredLBStrategy, Warning, TEXT("Load balancing strategy does not contain default layer. "
		"This is needed to render worker debug visualization. Class: %s"), *GetNameSafe(this));

	return nullptr;
}

FName ULayeredLBStrategy::GetLayerNameForClass(const TSubclassOf<AActor> Class) const
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
	return GetLayerNameForClass(ActorA->GetClass()) == GetLayerNameForClass(ActorB->GetClass());
}

FName ULayeredLBStrategy::GetLayerNameForActor(const AActor& Actor) const
{
	return GetLayerNameForClass(Actor.GetClass());
}

void ULayeredLBStrategy::AddStrategyForLayer(const FName& LayerName, UAbstractLBStrategy* LBStrategy)
{
	LayerNameToLBStrategy.Add(LayerName, LBStrategy);
	LayerNameToLBStrategy[LayerName]->Init(nullptr);
}
