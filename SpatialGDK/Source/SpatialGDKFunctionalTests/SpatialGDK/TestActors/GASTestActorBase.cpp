// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GASTestActorBase.h"

AGASTestActorBase::AGASTestActorBase()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
}

void AGASTestActorBase::BeginPlay()
{
	Super::BeginPlay();
	GrantInitialAbilitiesIfNeeded();
}

void AGASTestActorBase::OnAuthorityGained()
{
	Super::OnAuthorityGained();
	GrantInitialAbilitiesIfNeeded();
}

void AGASTestActorBase::GrantInitialAbilitiesIfNeeded()
{
	if (!bHasGrantedAbilities && HasAuthority())
	{
		for (const TSubclassOf<UGameplayAbility>& Ability : GetInitialGrantedAbilities())
		{
			FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));
			UE_LOG(LogTemp, Log, TEXT("Handle valid: %s"), Handle.IsValid() ? TEXT("true") : TEXT("false"));
		}

		bHasGrantedAbilities = true;
	}
}
