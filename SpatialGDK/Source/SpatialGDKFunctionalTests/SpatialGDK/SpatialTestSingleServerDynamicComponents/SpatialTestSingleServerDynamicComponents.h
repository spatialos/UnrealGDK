// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestSingleServerDynamicComponents.generated.h"

class ATestDynamicComponentActor;
class UTestDynamicComponent;

UCLASS()
class ASpatialTestSingleServerDynamicComponents : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestSingleServerDynamicComponents();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	ATestDynamicComponentActor* TestActor;

	UTestDynamicComponent* CreateAndAttachTestDynamicComponentToActor(AActor* Actor, FName Name);

	const FVector ActorSpawnPosition = FVector(0.0f, 0.0f, 50.0f);
};
