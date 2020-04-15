// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SpatialLayerManager.h"
#include "SpatialGDKSettings.h"

void SpatialLayerManager::Init()
{
}

FName SpatialLayerManager::GetLayerForClass(const TSubclassOf<AActor> Class)
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

FName SpatialLayerManager::GetWorkerTypeForClass(const TSubclassOf<AActor> Class)
{
	const FName Layer = GetLayerForClass(Class);

	if (const FName* WorkerType = LayerToWorkerType.Find(Layer))
	{
		return *WorkerType;
	}

	return DefaultWorkerType;
}

FName SpatialLayerManager::GetWorkerTypeForLayer(const FName& Layer) const
{
	if (const FName* WorkerType = LayerToWorkerType.Find(Layer))
	{
		return *WorkerType;
	}

	return DefaultWorkerType;
}

bool SpatialLayerManager::IsSameWorkerType(const AActor* ActorA, const AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return false;
	}

	const FName& WorkerTypeA = GetWorkerTypeForClass(ActorA->GetClass());
	const FName& WorkerTypeB = GetWorkerTypeForClass(ActorB->GetClass());

	return (WorkerTypeA == WorkerTypeB);
}
