// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "AlwaysInterestedTest.generated.h"

class AAlwaysInterestedTestActor;
class ASmallNCDActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AAlwaysInterestedTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	AAlwaysInterestedTest();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PrepareTest() override;

	UPROPERTY(Replicated)
	AAlwaysInterestedTestActor* AlwaysInterestedActor;

	UPROPERTY(Replicated)
	ASmallNCDActor* InterestedInThisReplicatedActor;

	UPROPERTY(Replicated)
	ASmallNCDActor* NotInterestedInThisReplicatedActor;
};
