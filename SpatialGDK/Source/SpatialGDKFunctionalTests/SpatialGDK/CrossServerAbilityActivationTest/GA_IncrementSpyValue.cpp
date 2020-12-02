// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GA_IncrementSpyValue.h"

UGA_IncrementSpyValue::UGA_IncrementSpyValue()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
}

void UGA_IncrementSpyValue::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
											const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		}
		UE_LOG(LogTemp, Log, TEXT("Activated ability"));
	}
}
