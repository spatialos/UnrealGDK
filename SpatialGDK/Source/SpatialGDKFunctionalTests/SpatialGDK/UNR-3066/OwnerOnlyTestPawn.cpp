// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "OwnerOnlyTestPawn.h"
#include "Net/UnrealNetwork.h"

void AOwnerOnlyTestPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AOwnerOnlyTestPawn, TestInt, COND_OwnerOnly);
}
