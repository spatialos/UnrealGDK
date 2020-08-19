// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialAuthorityTest.generated.h"

class ASpatialAuthorityTestActor;
class ASpatialAuthorityTestReplicatedActor;

/** Check SpatialAuthorityTest.cpp for Test explanation. */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialAuthorityTest();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void FinishStep() override { ResetTimer(); Super::FinishStep();	};

	void ResetTimer() {	Timer = 0.5; };

	bool VerifyTestActor(ASpatialAuthorityTestActor* Actor, int AuthorityOnBeginPlay, int AuthorityOnTick, int NumAuthorityGains, int NumAuthorityLosses);

	UFUNCTION(CrossServer, Reliable)
	void CrossServerSetDynamicReplicatedActor(ASpatialAuthorityTestReplicatedActor* Actor);

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialAuthorityTestActor* LevelActor;

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialAuthorityTestReplicatedActor* LevelReplicatedActor;

	// This needs to be a position that belongs to Server 1.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server1Position;

	// This needs to be a position that belongs to Server 2.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server2Position;

	UPROPERTY(Replicated)
	ASpatialAuthorityTestReplicatedActor* DynamicReplicatedActor;

	UPROPERTY()
	ASpatialAuthorityTestActor* DynamicNonReplicatedActor;


	float Timer;
	
};
