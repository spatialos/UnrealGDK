// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestMultiServerUnrealComponents.generated.h"

class ATestUnrealComponentsActor;

UCLASS()
class ASpatialTestMultiServerUnrealComponents : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestMultiServerUnrealComponents();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PrepareTest() override;

	void ProcessActorProperties(bool bWrite, bool bOwnerOnlyExpected = false, bool bHandoverExpected = false,
								bool bInitialOnlyExpected = false);

	UPROPERTY(Replicated)
	ATestUnrealComponentsActor* TestActor;

	const FVector ActorSpawnPosition = FVector(0.0f, 0.0f, 50.0f);
	bool bInitialOnlyEnabled;
};
