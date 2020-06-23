// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AutoDestroyComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SPATIALGDKFUNCTIONALTESTS_API UAutoDestroyComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UAutoDestroyComponent();
	
};
