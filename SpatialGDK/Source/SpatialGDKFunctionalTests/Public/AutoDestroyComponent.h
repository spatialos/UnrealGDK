// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoDestroyComponent.generated.h"


/*
* Empty component to be added to actors so that they can be automatically destroyed when the tests finish
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPATIALGDKFUNCTIONALTESTS_API UAutoDestroyComponent : public USceneComponent
{
	GENERATED_BODY()
};
