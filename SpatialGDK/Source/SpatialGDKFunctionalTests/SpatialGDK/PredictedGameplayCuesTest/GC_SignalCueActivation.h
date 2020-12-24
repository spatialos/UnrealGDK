// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "GC_SignalCueActivation.generated.h"

/**
 * TODO
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API UGC_SignalCueActivation : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UGC_SignalCueActivation();

	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;

	static FGameplayTag GetAddTag() { return FGameplayTag::RequestGameplayTag(FName(TEXT("GameplayCue.Add"))); }
	static FGameplayTag GetExecuteTag() { return FGameplayTag::RequestGameplayTag(FName(TEXT("GameplayCue.Execute"))); }
};
