// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GC_SignalCueActivation.h"
#include "CuesGASTestPawn.h"

UGC_SignalCueActivation::UGC_SignalCueActivation() {}

void UGC_SignalCueActivation::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType,
												const FGameplayCueParameters& Parameters)
{
	if (MyTarget == nullptr || !MyTarget->IsA<ACuesGASTestPawn>())
	{
		UE_LOG(LogSpatialFunctionalTest, Error, TEXT("GC_SignalCueActivation, target actor %s is not an ACuesGASTestPawn."), *GetNameSafe(MyTarget));
		return;
	}

	ACuesGASTestPawn* TestActor = static_cast<ACuesGASTestPawn*>(MyTarget);

	if (EventType == EGameplayCueEvent::OnActive)
	{
		TestActor->SignalOnActive();
	}
	else if (EventType == EGameplayCueEvent::Executed)
	{
		TestActor->SignalExecute();
	}
}
