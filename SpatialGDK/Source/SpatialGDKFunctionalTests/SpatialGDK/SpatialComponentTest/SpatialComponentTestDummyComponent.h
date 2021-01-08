// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "SpatialComponentTestDummyComponent.generated.h"

/*
 * Empty component to be added and removed from components so that the component callbacks can be tests
 */
UCLASS(NotBlueprintable, ClassGroup = SpatialFunctionalTest)
class SPATIALGDKFUNCTIONALTESTS_API USpatialComponentTestDummyComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USpatialComponentTestDummyComponent();
};
