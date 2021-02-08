// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SpatialFunctionalTest.h"

#include "SpatialTestAttachedComponentReplication.generated.h"

namespace SpatialTestAttachedComponentReplicationValues
{
constexpr int InitialValue = 0;
constexpr int ChangedValue = 100;
} // namespace SpatialTestAttachedComponentReplicationValues

UCLASS(BlueprintType)
class ASpatialTestAttachedComponentReplicationActor : public AActor
{
	GENERATED_BODY()

public:
	ASpatialTestAttachedComponentReplicationActor();
};

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class USpatialTestAttachedComponentReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USpatialTestAttachedComponentReplicationComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int ReplicatedValue = SpatialTestAttachedComponentReplicationValues::InitialValue;
};

UCLASS(BlueprintType)
class ASpatialTestAttachedComponentReplicationActorWithDefaultComponent : public ASpatialTestAttachedComponentReplicationActor
{
	GENERATED_BODY()

public:
	ASpatialTestAttachedComponentReplicationActorWithDefaultComponent();
};

UENUM(BlueprintType)
enum class ESpatialTestAttachedComponentReplicationType : uint8
{
	None,
	LevelPlaced,
	DynamicallySpawnedWithDynamicComponent,
	DynamicallySpawnedWithDefaultComponent,
};

UCLASS()
class ASpatialTestAttachedComponentReplication : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestAttachedComponentReplication();

	virtual void PrepareTest() override;

	UPROPERTY(EditAnywhere, Category = "Test Shit")
	ESpatialTestAttachedComponentReplicationType bIsWorkingWithSceneActor;

	UPROPERTY(EditAnywhere, Category = "Test Shit")
	TSoftObjectPtr<AActor> SceneActor;

private:
	float TimeRunningStep2 = 0.0f;

	UPROPERTY()
	ASpatialTestAttachedComponentReplicationActor* LevelPlacedActor;

	UPROPERTY()
	USpatialTestAttachedComponentReplicationComponent* AttachedComponent;
};
