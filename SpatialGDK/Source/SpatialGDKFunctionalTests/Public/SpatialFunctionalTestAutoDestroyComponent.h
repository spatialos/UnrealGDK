// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpatialFunctionalTestAutoDestroyComponent.generated.h"


/*
* Empty component to be added to actors so that they can be automatically destroyed when the tests finish
*/
UCLASS( NotBlueprintable, ClassGroup=SpatialFunctionalTest )
class SPATIALGDKFUNCTIONALTESTS_API USpatialFunctionalTestAutoDestroyComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USpatialFunctionalTestAutoDestroyComponent();
};
