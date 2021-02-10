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
// This value has to be zero as handover shadow state is zero-initialized
static constexpr int BasicTestPropertyValue = 0;
static constexpr int UpdatedTestPropertyValue = 100;

// Positions that belong to specific server according to 1x2 Grid LBS.
// Forward-Left, will be in Server 1's authority area.
static const FVector Server1Position{ 1000.0f, -1000.0f, 0.0f };

// Forward-Right, will be in Server 2's authority area.
static const FVector Server2Position{ 1000.0f, 1000.0f, 0.0f };
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
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestHandoverActorComponentReplication : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestHandoverActorComponentReplication();

	virtual void PrepareTest() override;

	void RequireHandoverCubeAuthorityAndPosition(int WorkerShouldHaveAuthority, const FVector& ExpectedPosition);

	UPROPERTY()
	AHandoverReplicationTestCube* HandoverCube;
};
