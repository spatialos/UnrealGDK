// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialFunctionalTest.h"

#include "Schema/LoadBalancingStuff.h"

#include "LoadBalancing/SpatialMultiWorkerSettings.h"

#include "TestMaps/GeneratedTestMap.h"

#include "SpatialTestLoadBalancingData.generated.h"

UCLASS()
class USpatialTestLoadBalancingDataTestMap : public UGeneratedTestMap
{
	GENERATED_BODY()

	USpatialTestLoadBalancingDataTestMap()
		: Super(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("SpatialTestLoadBalancingData"))
	{
	}

	virtual void CreateCustomContentForMap() override;
};

UCLASS()
class USpatialTestLoadBalancingDataMultiWorkerSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

	USpatialTestLoadBalancingDataMultiWorkerSettings();
};

UCLASS()
class ASpatialTestLoadBalancingDataActor : public AActor
{
	GENERATED_BODY()
public:
	ASpatialTestLoadBalancingDataActor();
};

UCLASS()
class ASpatialTestLoadBalancingDataOffloadedActor : public AActor
{
	GENERATED_BODY()
public:
	ASpatialTestLoadBalancingDataOffloadedActor();
};

UCLASS()
class ASpatialTestLoadBalancingData : public ASpatialFunctionalTest
{
	GENERATED_BODY()

	virtual void PrepareTest() override;

	TOptional<SpatialGDK::LoadBalancingData> GetLoadBalancingData(const AActor* Actor) const;

	TWeakObjectPtr<ASpatialTestLoadBalancingDataActor> TargetActor;
	TWeakObjectPtr<ASpatialTestLoadBalancingDataOffloadedActor> TargetOffloadedActor;
};
