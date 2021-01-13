// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GA_IncrementSpyValue.h"
#include "GameplayTagContainer.h"
#include "LogSpatialFunctionalTest.h"
#include "SpyValueGASTestActor.h"

UGA_IncrementSpyValue::UGA_IncrementSpyValue()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	AbilityTags.AddTag(UGA_IncrementSpyValue::GetTriggerTag());

	FAbilityTriggerData Trigger;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	Trigger.TriggerTag = UGA_IncrementSpyValue::GetTriggerTag();
	AbilityTriggers.Add(Trigger);
}

void UGA_IncrementSpyValue::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
											const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
			return;
		}

		AActor* TestActor = ActorInfo->OwnerActor.Get();
		if (ASpyValueGASTestActor* SpyValueActor = Cast<ASpyValueGASTestActor>(ActorInfo->OwnerActor.Get()))
		{
			SpyValueActor->IncrementCounter();
		}
		else
		{
			UE_LOG(LogSpatialFunctionalTest, Error, TEXT("UGA_IncrementSpyValue was activated with an invalid owner actor %s."),
				   *GetNameSafe(TestActor));
		}

		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
	}
}
