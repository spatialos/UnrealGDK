// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestReplicatedStartupActor.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestReplicatedStartupActor : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestReplicatedStartupActor();

	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	bool bIsValidReference;

	AActor* ReplicatedActor;
};
