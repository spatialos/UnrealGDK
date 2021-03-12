// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDynamicComponent.h"

#include "Net/UnrealNetwork.h"

UTestDynamicComponent::UTestDynamicComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UTestDynamicComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ReferencesArray);
	DOREPLIFETIME_CONDITION(ThisClass, OwnerOnlyReplicatedVar, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, InitialOnlyReplicatedVar, COND_InitialOnly);
}
