// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Abilities/GameplayAbility.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GA_IncrementSpyValue.generated.h"

/**
 * Calls `IncrementCounter()` on its owner to signal that this ability ran. The owner must be an ASpyValueGASTestActor
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UGA_IncrementSpyValue : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_IncrementSpyValue();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	static FGameplayTag GetTriggerTag() { return FGameplayTag::RequestGameplayTag(TEXT("GameplayAbility.Trigger")); }
};
