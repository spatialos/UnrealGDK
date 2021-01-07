// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GC_SignalCueActivation.h"
#include "CuesGASTestActor.h"

UGC_SignalCueActivation::UGC_SignalCueActivation() {}

void UGC_SignalCueActivation::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType,
												const FGameplayCueParameters& Parameters)
{
	ACuesGASTestActor* TestActor = static_cast<ACuesGASTestActor*>(MyTarget);
	if (TestActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("GC_SignalCueActivation, target actor %s is not an ACuesGASTestActor."), *GetNameSafe(MyTarget));
		return;
	}

	if (EventType == EGameplayCueEvent::OnActive)
	{
		TestActor->SignalOnActive();
	}
	else if (EventType == EGameplayCueEvent::Executed)
	{
		TestActor->SignalExecute();
	}
}
