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

	DOREPLIFETIME(UTestDynamicComponent, ReferencesArray);
}
