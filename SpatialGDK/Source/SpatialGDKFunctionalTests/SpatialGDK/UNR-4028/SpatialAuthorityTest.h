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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	UPROPERTY(Replicated)
	TArray<int> GameModeServerAuthorities;

	UPROPERTY(Replicated)
	TArray<int> GameStateServerAuthorities;

	UPROPERTY(Replicated)
	TArray<int> GameStateClientAuthorities;

	virtual void StartTest() override;

	UFUNCTION(CrossServer, Reliable)
	void CrossServerSetDynamicReplicatedActor(ASpatialAuthorityTestReplicatedActor* Actor);

	UFUNCTION(CrossServer, Reliable)
	void CrossServerSetGameModeAuthorityFromServerWorker(int ServerWorkerId, int Authority);

	UFUNCTION(CrossServer, Reliable)
	void CrossServerSetGameStateAuthorityFromWorker(const FWorkerDefinition& WorkerDefinition, int Authority);

private:
	float Timer;
	
};
