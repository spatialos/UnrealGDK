// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GE_ExecuteCue.h"
#include "GC_SignalCueActivation.h"

UGE_ExecuteCue::UGE_ExecuteCue()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	GameplayCues.Add(FGameplayEffectCue(UGC_SignalCueActivation::GetExecuteTag(), 0.0f, 0.0f));
}
