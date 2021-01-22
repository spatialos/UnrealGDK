// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "LoadBalancing/GridBasedLBStrategy.h"
#include "LoadBalancing/SpatialMultiWorkerSettings.h"
#include "SpatialFunctionalTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3761/SpatialTestHandover/HandoverCube.h"
#include "Utils/LayerInfo.h"

#include "SpatialTestHandoverReplication.generated.h"

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

class ASpatialTestHandoverReplication;

UCLASS()
class AHandoverReplicationTestCube : public AHandoverCube
{
	GENERATED_BODY()

public:
	AHandoverReplicationTestCube();

	void SetTestValues(int UpdatedTestPropertyValue);

	void RequireTestValues(ASpatialTestHandoverReplication* FunctionalTest, int RequiredValue, const FString& Postfix) const;

	virtual void OnAuthorityGained() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Handover)
	int HandoverTestProperty = HandoverReplicationTestValues::BasicTestPropertyValue;

	UPROPERTY(Replicated)
	int ReplicatedTestProperty = HandoverReplicationTestValues::BasicTestPropertyValue;

	UPROPERTY(Handover)
	EHandoverReplicationTestStage ShouldResetValueToDefaultCounter = EHandoverReplicationTestStage::Initial;

	UPROPERTY()
	UTestHandoverComponent* HandoverComponent;
};

UCLASS()
class USpatialTestHandoverReplicationLBStrategy : public UGridBasedLBStrategy
{
	GENERATED_BODY()

	USpatialTestHandoverReplicationLBStrategy()
	{
		Rows = 2;
		Cols = 1;
		WorldWidth = HandoverReplicationTestValues::WorldSize;
		WorldHeight = HandoverReplicationTestValues::WorldSize;
	}
};

UCLASS(BlueprintType)
class SPATIALGDKFUNCTIONALTESTS_API USpatialTestHandoverReplicationMultiWorkerSettings : public USpatialMultiWorkerSettings
{
public:
	GENERATED_BODY()

	static TArray<FLayerInfo> GetLayerSetup()
	{
		const FLayerInfo GridLayer(TEXT("Grid"), { AActor::StaticClass() }, USpatialTestHandoverReplicationLBStrategy::StaticClass());

		return { GridLayer };
	}

	USpatialTestHandoverReplicationMultiWorkerSettings() { WorkerLayers.Append(GetLayerSetup()); }
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestHandoverReplication : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestHandoverReplication();

	virtual void PrepareTest() override;

	UPROPERTY(Handover)
	AHandoverReplicationTestCube* HandoverCube;

	void RequireHandoverCubeAuthorityAndPosition(int WorkerShouldHaveAuthority, const FVector& ExpectedPosition);

	bool MoveHandoverCube(const FVector& Position);

	// Positions that belong to specific server according to 1x2 Grid LBS.
	FVector Server1Position;
	FVector Server2Position;
};
