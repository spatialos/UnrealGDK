// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "SpatialFunctionalTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3761/SpatialTestHandover/HandoverCube.h"
#include "Utils/LayerInfo.h"

#include "SpatialTestHandoverActorComponentReplication.generated.h"

UENUM()
enum class EHandoverReplicationTestStage
{
	Initial,
	ChangeValuesToDefaultOnGainingAuthority,
	Final,
};

namespace HandoverReplicationTestValues
{
static constexpr int BasicTestPropertyValue = 10;
static constexpr int UpdatedTestPropertyValue = 100;

static constexpr float WorldSize = 1000.0f;

// Positions that belong to specific server according to 1x2 Grid LBS.
// Forward-Left, will be in Server 1's authority area.
static const FVector Server1Position{ WorldSize / 4, -WorldSize / 4, 0.0f };

// Forward-Right, will be in Server 2's authority area.
static const FVector Server2Position{ WorldSize / 4, WorldSize / 4, 0.0f };
} // namespace HandoverReplicationTestValues

UCLASS()
class UTestHandoverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTestHandoverComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Handover)
	int HandoverTestProperty = HandoverReplicationTestValues::BasicTestPropertyValue;

	UPROPERTY(Replicated)
	int ReplicatedTestProperty = HandoverReplicationTestValues::BasicTestPropertyValue;
};

class ASpatialTestHandoverActorComponentReplication;

UCLASS()
class AHandoverReplicationTestCube : public AHandoverCube
{
	GENERATED_BODY()

public:
	AHandoverReplicationTestCube();

	void SetTestValues(int UpdatedTestPropertyValue);

	void RequireTestValues(ASpatialTestHandoverActorComponentReplication* FunctionalTest, int RequiredValue, const FString& Postfix) const;

	virtual void OnAuthorityGained() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Handover)
	int HandoverTestProperty = HandoverReplicationTestValues::BasicTestPropertyValue;

	UPROPERTY(Replicated)
	int ReplicatedTestProperty = HandoverReplicationTestValues::BasicTestPropertyValue;

	UPROPERTY(Handover)
	EHandoverReplicationTestStage TestStage = EHandoverReplicationTestStage::Initial;

	UPROPERTY()
	UTestHandoverComponent* HandoverComponent;
};

UCLASS()
class USpatialTestHandoverReplicationLBStrategy : public UGridBasedLBStrategy
{
	GENERATED_BODY()

	USpatialTestHandoverReplicationLBStrategy()
	{
		Rows = 1;
		Cols = 2;
		WorldWidth = HandoverReplicationTestValues::WorldSize;
		WorldHeight = HandoverReplicationTestValues::WorldSize;
	}
};

UCLASS(BlueprintType)
class SPATIALGDKFUNCTIONALTESTS_API USpatialTestHandoverReplicationMultiWorkerSettings : public USpatialMultiWorkerSettings
{
public:
	GENERATED_BODY()

	USpatialTestHandoverReplicationMultiWorkerSettings()
	{
		WorkerLayers[0].LoadBalanceStrategy = USpatialTestHandoverReplicationLBStrategy::StaticClass();
	}
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestHandoverActorComponentReplication : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestHandoverActorComponentReplication();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PrepareTest() override;

	UFUNCTION(CrossServer, Reliable)
	void SaveHandoverCube(AHandoverReplicationTestCube* InHandoverCube);

	void RequireHandoverCubeAuthorityAndPosition(int WorkerShouldHaveAuthority, const FVector& ExpectedPosition);

	UPROPERTY(Replicated)
	AHandoverReplicationTestCube* HandoverCube;
};
