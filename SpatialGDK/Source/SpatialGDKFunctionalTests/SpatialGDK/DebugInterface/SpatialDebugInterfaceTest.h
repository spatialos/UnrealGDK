// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"
#include "SpatialFunctionalTest.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "TestMaps/GeneratedTestMap.h"
#include "SpatialDebugInterfaceTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialDebugInterfaceTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialDebugInterfaceTest();

	virtual void PrepareTest() override;

protected:
	bool WaitToSeeActors(UClass* ActorClass, int32 NumActors);

	TArray<VirtualWorkerId> Workers;
	VirtualWorkerId LocalWorker;
	FVector WorkerEntityPosition;
	bool bIsOnDefaultLayer = false;
	int32 DelegationStep = 0;
	int64 TimeStampSpinning;
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialDebugInterfaceMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialDebugInterfaceMap()
		: UGeneratedTestMap(EMapCategory::CI_PREMERGE, TEXT("SpatialDebugInterfaceMap"))
	{
	}


protected:
	virtual void CreateCustomContentForMap() override;
};

/**
 * A 2 by 1 (rows by columns) load balancing strategy for testing zoning features.
 * Has a world-wide interest border, so everything should be in view.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest1x2GridNoInterestStrategy : public UGridBasedLBStrategy
{
	GENERATED_BODY()

public:
	UTest1x2GridNoInterestStrategy()
	{
		Cols = 2;
	}
};


/**
 * Uses the Test1x2GridStrategy, otherwise has default settings.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UTest1x2WorkerNoInterestSettings : public USpatialMultiWorkerSettings
{
	GENERATED_BODY()

public:
	UTest1x2WorkerNoInterestSettings()
	{
		WorkerLayers[0].LoadBalanceStrategy = UTest1x2GridNoInterestStrategy::StaticClass();
	}
};
