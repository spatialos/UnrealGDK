// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialComponentTest.generated.h"

class ASpatialComponentTestActor;
class ASpatialComponentTestReplicatedActor;
enum class ESpatialHasAuthority : uint8;

/** Check SpatialComponentTest.cpp for Test explanation. */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialComponentTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialComponentTest();

	virtual void PrepareTest() override;


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void FinishStep() override
	{
		ResetTimer();
		Super::FinishStep();
	};

	void ResetTimer() { Timer = 0.5; };

	UFUNCTION(CrossServer, Reliable)
	void CrossServerSetDynamicReplicatedActor(ASpatialComponentTestReplicatedActor* Actor);

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialComponentTestActor* LevelActor;

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialComponentTestReplicatedActor* LevelReplicatedActor;

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialComponentTestReplicatedActor* LevelReplicatedActorOnBorder;

	// This needs to be a position that belongs to Server 1.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server1Position;

	// This needs to be a position that belongs to Server 2.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server2Position;

	// This needs to be a position that belongs to Server 3.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server3Position;

	// This needs to be a position that belongs to Server 4.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server4Position;

	// This needs to be a position on the border between all servers.
	FVector BorderPosition;

	UPROPERTY(Replicated)
	ASpatialComponentTestReplicatedActor* DynamicReplicatedActor;

	UPROPERTY()
	ASpatialComponentTestActor* DynamicNonReplicatedActor;

	// Local timer used for some active waits.
	float Timer;
};
