// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GC_SignalCueActivation.h"
#include "CuesGASTestActor.h"

UGC_SignalCueActivation::UGC_SignalCueActivation() {}

void UGC_SignalCueActivation::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType,
												const FGameplayCueParameters& Parameters)
{
	UE_LOG(LogTemp, Log, TEXT("Cue event happened, type %d"), EventType);

	ACuesGASTestActor* TestActor = static_cast<ACuesGASTestActor*>(MyTarget);
	if (TestActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("GC_SignalCueActivation, target actor %s is not an ACuesGASTestActor."), *GetNameSafe(MyTarget));
		return;
	}

	if (EventType == EGameplayCueEvent::Executed)
	{
		TestActor->ExecutedCue();
	}
}
