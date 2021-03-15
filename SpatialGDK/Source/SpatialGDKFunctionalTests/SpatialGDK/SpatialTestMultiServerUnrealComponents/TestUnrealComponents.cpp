// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestUnrealComponents.h"

#include "Net/UnrealNetwork.h"

UDataOnlyComponent::UDataOnlyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UDataOnlyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DataReplicatedVar);
}

UDataAndHandoverComponent::UDataAndHandoverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UDataAndHandoverComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DataReplicatedVar);
}

UDataAndOwnerOnlyComponent::UDataAndOwnerOnlyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UDataAndOwnerOnlyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DataReplicatedVar);
	DOREPLIFETIME_CONDITION(ThisClass, OwnerOnlyReplicatedVar, COND_OwnerOnly);
}

UDataAndInitialOnlyComponent::UDataAndInitialOnlyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UDataAndInitialOnlyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DataReplicatedVar);
	DOREPLIFETIME_CONDITION(ThisClass, InitialOnlyReplicatedVar, COND_InitialOnly);
}
