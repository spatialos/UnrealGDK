// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Abilities/GameplayAbility.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GA_ApplyCueEffect.generated.h"

/**
 * Local-predicted, non-instanced ability, set up to be triggered by UGC_SignalCueActivation's tags through gameplay events.
 * Depending on which tag the ability is triggered with, it applies either the UGE_AddCue or the UGE_ExecuteCue gameplay effect to its
 * owner, which then trigger events on UGC_SignalCueActivation.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UGA_ApplyCueEffect : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_ApplyCueEffect();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
