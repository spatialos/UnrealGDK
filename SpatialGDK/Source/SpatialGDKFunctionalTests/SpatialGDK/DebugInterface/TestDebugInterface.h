// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"
#include "SpatialFunctionalTest.h"
#include "TestDebugInterface.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ATestDebugInterface : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ATestDebugInterface();

	virtual void BeginPlay() override;

protected:
	bool WaitToSeeActors(UClass* ActorClass, int32 NumActors);

	TArray<VirtualWorkerId> Workers;
	VirtualWorkerId LocalWorker;
	FVector WorkerEntityPosition;
	bool bIsOnDefaultLayer = false;
	int32 DelegationStep = 0;
	int64 TimeStampSpinning;
};
