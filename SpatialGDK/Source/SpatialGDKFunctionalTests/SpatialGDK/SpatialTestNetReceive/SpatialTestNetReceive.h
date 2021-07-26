// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "SpatialTestNetReceive.generated.h"

class ASpatialTestNetReceiveActor;
class USpatialTestNetReceiveSubobject;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestNetReceive : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestNetReceive();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PrepareTest() override;

	UPROPERTY(Replicated)
	ASpatialTestNetReceiveActor* TestActor;
};

UCLASS()
class ASpatialTestNetReceiveActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	ASpatialTestNetReceiveActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	USpatialTestNetReceiveSubobject* Subobject;
};

enum class RepStep
{
	PreRep,
	PreNetReceive,
	PostNetReceive,
	RepNotify,

	SecondPreNetReceive, // For this test, expected to have RepNotify called before it
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialTestNetReceiveSubobject : public UActorComponent
{
	GENERATED_BODY()

public:
	USpatialTestNetReceiveSubobject();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void PreNetReceive() override;
	void PostNetReceive() override;

	UPROPERTY(ReplicatedUsing = OnRep_TestInt)
	int32 TestInt;

	// Use an owner only test int to ensure two spatial components are added and updated
	UPROPERTY(Replicated)
	int32 OwnerOnlyTestInt;

	int32 PreNetNumTimesCalled;
	int32 PostNetNumTimesCalled;
	int32 RepNotifyNumTimesCalled;

	RepStep PreviousReplicationStep = RepStep::PreRep;
	TMap<RepStep, RepStep> RepStepToPrevRepStep;

	UFUNCTION()
	void OnRep_TestInt(int32 OldTestInt);
};
