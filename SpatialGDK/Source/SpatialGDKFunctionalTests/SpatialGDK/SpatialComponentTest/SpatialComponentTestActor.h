// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpatialComponentTestActor.generated.h"

class USpatialComponentTestCallbackComponent;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialComponentTestActor : public AActor
{
	GENERATED_BODY()

public:
	ASpatialComponentTestActor();

	UPROPERTY()
	USpatialComponentTestCallbackComponent* CallbackComponent;
};
