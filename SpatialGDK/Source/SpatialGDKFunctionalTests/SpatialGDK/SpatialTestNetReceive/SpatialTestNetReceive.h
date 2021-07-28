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

UENUM()
enum class ERepStep
{
	None,
	PreRep,
	PreNetReceive,
	PostNetReceive,
	RepNotify,
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

	TArray<ERepStep> RepSteps;
	TArray<ERepStep> ExpectedRepSteps = { ERepStep::PreNetReceive, ERepStep::PostNetReceive, ERepStep::RepNotify, ERepStep::PreNetReceive,
										  ERepStep::PostNetReceive };
	// The number of steps from the start of ExpectedRepSteps that are mandatory, where the test fails if they don't exist.
	// Past this number, if we don't receive the expected repstep (and instead receive nothing), the test can still pass.
	static constexpr int32 NumMandatorySteps = 3;

	UFUNCTION()
	void OnRep_TestInt();
};
