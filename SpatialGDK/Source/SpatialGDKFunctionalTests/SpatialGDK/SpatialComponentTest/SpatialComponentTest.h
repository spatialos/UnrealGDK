// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

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

	UFUNCTION(CrossServer, Reliable)
	void CrossServerSetDynamicReplicatedActor(ASpatialComponentTestReplicatedActor* Actor);

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialComponentTestActor* LevelActor;

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialComponentTestReplicatedActor* LevelReplicatedActor;

	// This needs to be a position that belongs to Server 1.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server1Position;

	// This needs to be a position that belongs to Server 2.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server2Position;

	UPROPERTY(Replicated)
	ASpatialComponentTestReplicatedActor* DynamicReplicatedActor;

private:
	void CheckComponentsOnServer(ASpatialComponentTestActor* Actor, int ExpectedServerId);
	void CheckComponentsCrossServer(ASpatialComponentTestActor* Actor, int StartServerId, int EndServerId);
	bool VerifyServerComponents(ASpatialComponentTestActor* Actor, int ExpectedComponentCount = 0, int NumAuthorityGains = 0,
								int NumAuthorityLosses = 0, int NumActorReadyAuth = 0, int NumActorReadyNonAuth = 0);
	bool VerifyClientComponents(ASpatialComponentTestActor* Actor, int ExpectedComponentCount = 0, int NumClientOwnershipGained = 0,
								int NumClientOwnershipLost = 0);

	TArray<UActorComponent*> GetDynamicComponents(ASpatialComponentTestActor* Actor);
};
