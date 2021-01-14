// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CuesGASTestPawn.h"
#include "GA_ApplyCueEffect.h"

ACuesGASTestPawn::ACuesGASTestPawn()
	: OnActiveCounter(0)
	, ExecuteCounter(0)
{
}

TArray<TSubclassOf<UGameplayAbility>> ACuesGASTestPawn::GetInitialGrantedAbilities()
{
	return { UGA_ApplyCueEffect::StaticClass() };
}
