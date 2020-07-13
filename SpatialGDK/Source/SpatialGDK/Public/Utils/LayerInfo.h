// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "LayerInfo.generated.h"

class UAbstractLBStrategy;
class UAbstractLockingPolicy;

USTRUCT()
struct FLayerInfo
{
	GENERATED_BODY()

	FLayerInfo() : Name(NAME_None)
	{
	}

	FLayerInfo(FName InName, TSet<TSoftClassPtr<AActor>> InActorClasses, TSubclassOf<UAbstractLBStrategy> InLoadBalanceStrategy)
		: Name(InName)
		, ActorClasses(InActorClasses)
		, LoadBalanceStrategy(InLoadBalanceStrategy) {}

	UPROPERTY()
	FName Name;

	// Using TSoftClassPtr here to prevent eagerly loading all classes.
	/** The Actor classes contained within this group. Children of these classes will also be included. */
	UPROPERTY(EditAnywhere, Category = "SpatialGDK")
	TSet<TSoftClassPtr<AActor>> ActorClasses;

	UPROPERTY(EditAnywhere, Category = "Load Balancing")
	TSubclassOf<UAbstractLBStrategy> LoadBalanceStrategy;
};
