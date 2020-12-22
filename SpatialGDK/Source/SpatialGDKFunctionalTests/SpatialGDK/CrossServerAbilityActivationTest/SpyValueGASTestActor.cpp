// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpyValueGASTestActor.h"
#include "GA_IncrementSpyValue.h"
#include "Net/UnrealNetwork.h"

ASpyValueGASTestActor::ASpyValueGASTestActor()
	: Counter(0)
{
}

void ASpyValueGASTestActor::OnAuthorityGained()
{
	Super::OnAuthorityGained();
	UE_LOG(LogTemp, Log, TEXT("Auth gained on target actor."));
}

void ASpyValueGASTestActor::IncrementCounter()
{
	Counter++;
}

int ASpyValueGASTestActor::GetCounter()
{
	return Counter;
}

void ASpyValueGASTestActor::ResetCounter_Implementation()
{
	Counter = 0;
}

TArray<TSubclassOf<UGameplayAbility>> ASpyValueGASTestActor::GetInitialGrantedAbilities()
{
	return { UGA_IncrementSpyValue::StaticClass() };
}

void ASpyValueGASTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASpyValueGASTestActor, Counter);
}
