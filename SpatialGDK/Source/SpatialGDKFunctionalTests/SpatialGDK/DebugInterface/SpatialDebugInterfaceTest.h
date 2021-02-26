// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "SpatialCommonTypes.h"
#include "SpatialFunctionalTest.h"
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

