// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenObjectStub.h"

#include "Net/UnrealNetwork.h"

void USchemaGenObjectStub::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USchemaGenObjectStub, IntValue);
	DOREPLIFETIME(USchemaGenObjectStub, BoolValue);
}
