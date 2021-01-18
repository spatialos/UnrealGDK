// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GA_ApplyCueEffect.h"
#include "GC_SignalCueActivation.h"
#include "GE_AddCue.h"
#include "GE_ExecuteCue.h"
#include "GameplayTagContainer.h"

UGA_ApplyCueEffect::UGA_ApplyCueEffect()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FAbilityTriggerData AddTrigger;
	AddTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AddTrigger.TriggerTag = UGC_SignalCueActivation::GetAddTag();
	AbilityTriggers.Add(AddTrigger);

	FAbilityTriggerData ExecuteTrigger;
	ExecuteTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	ExecuteTrigger.TriggerTag = UGC_SignalCueActivation::GetExecuteTag();
	AbilityTriggers.Add(ExecuteTrigger);
}

void UGA_ApplyCueEffect::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
										 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, true);
			return;
		}

		if (TriggerEventData->EventTag == UGC_SignalCueActivation::GetAddTag())
		{
			ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, UGE_AddCue::StaticClass()->GetDefaultObject<UGameplayEffect>(),
									   0.0f);
		}
		else if (TriggerEventData->EventTag == UGC_SignalCueActivation::GetExecuteTag())
		{
			ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo,
									   UGE_ExecuteCue::StaticClass()->GetDefaultObject<UGameplayEffect>(), 0.0f);
		}

		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
	}
}
