// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "DormancyTestActor.generated.h"

/**
 * A Helper actor for the dormancy tests.
 * Has a TestIntProp to see if it replicates when it should/shouldn't.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADormancyTestActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	ADormancyTestActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int TestIntProp;
};
