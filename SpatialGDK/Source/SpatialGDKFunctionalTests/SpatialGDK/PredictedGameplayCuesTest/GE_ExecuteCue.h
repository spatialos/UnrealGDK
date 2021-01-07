// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameplayEffect.h"
#include "GE_ExecuteCue.generated.h"

/**
 * Instant gameplay effect that triggers UGC_SignalCueActivation.
 * UGC_SignalCueActivation will receive an Execute event (as well as an OnRemove event on the predicting client when the predicted effect
 * instance is removed.)
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UGE_ExecuteCue : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UGE_ExecuteCue();
};
