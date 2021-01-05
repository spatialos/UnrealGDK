// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CuesGASTestActor.h"
#include "GA_ApplyCueEffect.h"

ACuesGASTestActor::ACuesGASTestActor()
	: ExecuteCounter(0)
	, AddCounter(0)
{
}

void ACuesGASTestActor::OnRep_Owner()
{
	UE_LOG(LogTemp, Log, TEXT("gas test actor received owner %s"), *GetNameSafe(GetOwner()));
	GetAbilitySystemComponent()->RefreshAbilityActorInfo();
}

void ACuesGASTestActor::OnActorReady(bool bHasAuthority)
{
	if (bHasAuthority)
	{
		SetAutonomousProxy(true);
	}
}

TArray<TSubclassOf<UGameplayAbility>> ACuesGASTestActor::GetInitialGrantedAbilities()
{
	return { UGA_ApplyCueEffect::StaticClass() };
}
