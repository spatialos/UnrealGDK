// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GE_AddCue.h"
#include "AttributeSet.h"
#include "GC_SignalCueActivation.h"

UGE_AddCue::UGE_AddCue()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(2.0f)); // Fixed 2 seconds duration

	GameplayCues.Add(FGameplayEffectCue(UGC_SignalCueActivation::GetAddTag(), 0.0f, 0.0f));
}
