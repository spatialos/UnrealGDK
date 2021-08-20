// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "RefreshActorDormancyTestActor.h"
#include "RefreshActorDormancyTest.generated.h"

class ADormancyTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ARefreshActorDormancyTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	ARefreshActorDormancyTest();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PrepareTest() override;

private:
	UPROPERTY(Replicated)
	ARefreshActorDormancyTestActor* DormantToAwakeActor;

	UPROPERTY(Replicated)
	ARefreshActorDormancyTestActor* AwakeToDormantActor;
};
