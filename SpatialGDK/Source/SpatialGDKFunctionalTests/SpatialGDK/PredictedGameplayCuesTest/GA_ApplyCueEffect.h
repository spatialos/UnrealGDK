// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Abilities/GameplayAbility.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GA_ApplyCueEffect.generated.h"

/**
 * TODO
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
