// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameplayEffect.h"
#include "GE_AddCue.generated.h"

/**
 * Two-second long gameplay effect that triggers UGC_SignalCueActivation.
 * By having a duration, this effect will trigger OnActive/WhileActive/Removed events on the gameplay cue.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UGE_AddCue : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_AddCue();
};
