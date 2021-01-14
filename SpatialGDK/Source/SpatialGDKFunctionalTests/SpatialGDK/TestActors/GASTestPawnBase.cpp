// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GASTestPawnBase.h"

AGASTestPawnBase::AGASTestPawnBase()
{
	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
}

void AGASTestPawnBase::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GrantInitialAbilitiesIfNeeded();
	}
}

void AGASTestPawnBase::OnAuthorityGained()
{
	Super::OnAuthorityGained();
	GrantInitialAbilitiesIfNeeded();
}

void AGASTestPawnBase::GrantInitialAbilitiesIfNeeded()
{
	if (!bHasGrantedAbilities)
	{
		for (const TSubclassOf<UGameplayAbility>& Ability : GetInitialGrantedAbilities())
		{
			FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));
		}

		bHasGrantedAbilities = true;
	}
}

void AGASTestPawnBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AbilitySystemComponent->RefreshAbilityActorInfo();
}

void AGASTestPawnBase::OnRep_Controller()
{
	Super::OnRep_Controller();
	AbilitySystemComponent->RefreshAbilityActorInfo();
}
