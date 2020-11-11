// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialCommonTypes.h"
#include "SpatialFunctionalTest.h"

#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerController.h"

#include "SpatialTestPlayerControllerHandover.generated.h"

class ULayeredLBStrategy;

UCLASS()
class ASpatialTestPlayerControllerHandoverGameMode : public AGameModeBase
{
	GENERATED_UCLASS_BODY()
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestPlayerControllerHandover : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestPlayerControllerHandover();

	virtual void PrepareTest() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	using ASpatialFunctionalTest::SetTagDelegation;

	APlayerController* GetPlayerController();

	UFUNCTION()
	void OnRep_DestinationWorker();

	// The Load Balancing used by the test, needed to decide what Server should have authority over the HandoverCube.
	ULayeredLBStrategy* LoadBalancingStrategy;

	TMap<VirtualWorkerId, FVector> WorkerPositions;
	VirtualWorkerId LocalWorker;
	bool bIsOnDefaultLayer;

	UPROPERTY(ReplicatedUsing = OnRep_DestinationWorker)
	int32 DestinationWorker;

	float CheckNoPlayerSpawnTime;

	bool bReceivedNewDestination;
};
