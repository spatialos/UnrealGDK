// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpyValueGASTestActor.h"
#include "GA_IncrementSpyValue.h"

ASpyValueGASTestActor::ASpyValueGASTestActor() {}

void ASpyValueGASTestActor::OnAuthorityGained()
{
	Super::OnAuthorityGained();
	UE_LOG(LogTemp, Log, TEXT("Auth gained on target actor."));
}

TArray<TSubclassOf<UGameplayAbility>> ASpyValueGASTestActor::GetInitialGrantedAbilities()
{
	return { UGA_IncrementSpyValue::StaticClass() };
}
