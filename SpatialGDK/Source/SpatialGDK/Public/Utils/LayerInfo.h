// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "LoadBalancing/GridBasedLBStrategy.h"

#include "CoreMinimal.h"

#include "LayerInfo.generated.h"

class UAbstractLBStrategy;
class UAbstractLockingPolicy;

USTRUCT()
struct FLayerInfo
{
	GENERATED_BODY()

	FLayerInfo()
		: Name(TEXT("New Layer"))
		, ActorClasses({})
		, LoadBalanceStrategy(USingleWorkerStrategy::StaticClass())
	{
	}

	FLayerInfo(FName InName, TSet<TSoftClassPtr<AActor>> InActorClasses, UClass* InLoadBalanceStrategy)
		: Name(InName)
		, ActorClasses(InActorClasses)
		, LoadBalanceStrategy(InLoadBalanceStrategy)
	{
	}

	UPROPERTY(EditAnywhere, Category = "Load Balancing")
	FName Name;

	// Using TSoftClassPtr here to prevent eagerly loading all classes.
	/** The Actor classes contained within this group. Children of these classes will also be included. */
	UPROPERTY(EditAnywhere, Category = "Load Balancing")
	TSet<TSoftClassPtr<AActor>> ActorClasses;

	UPROPERTY(EditAnywhere, Category = "Load Balancing")
	TSubclassOf<UAbstractLBStrategy> LoadBalanceStrategy;
};
