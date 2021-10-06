// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "RelevancyTest.generated.h"

class AAlwaysRelevantTestActor;
class AAlwaysRelevantServerOnlyTestActor;
class AOnlyRelevantToOwnerTestActor;
class AUseOwnerRelevancyTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ARelevancyTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ARelevancyTest();

	virtual void PrepareTest() override;

	UPROPERTY()
	AAlwaysRelevantTestActor* AlwaysRelevantActor;

	UPROPERTY()
	AAlwaysRelevantServerOnlyTestActor* AlwaysRelevantServerOnlyActor;

	UPROPERTY()
	AOnlyRelevantToOwnerTestActor* OnlyRelevantToOwnerTestActor;

	UPROPERTY()
	AUseOwnerRelevancyTestActor* UseOwnerRelevancyTestActor;

	UPROPERTY()
	TArray<AActor*> ActiveActors;
};
