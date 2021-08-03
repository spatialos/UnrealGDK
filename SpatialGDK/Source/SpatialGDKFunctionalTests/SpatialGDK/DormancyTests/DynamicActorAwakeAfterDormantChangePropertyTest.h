// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "DormancyTest.h"
#include "DynamicActorAwakeAfterDormantChangePropertyTest.generated.h"


class ADormancyTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADynamicActorAwakeAfterDormantChangePropertyTest : public ADormancyTest
{
	GENERATED_BODY()

public:
	ADynamicActorAwakeAfterDormantChangePropertyTest();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PrepareTest() override;

private:
	UPROPERTY(Replicated)
	ADormancyTestActor* DormancyActor;
};
