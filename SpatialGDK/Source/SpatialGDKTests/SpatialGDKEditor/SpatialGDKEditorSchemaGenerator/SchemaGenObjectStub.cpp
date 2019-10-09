// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenObjectStub.h"

#include "Net/UnrealNetwork.h"

FRPCErrorInfo USchemaGenObjectStub::ProcessRPC(const FPendingRPCParams& Params)
{
	return FRPCErrorInfo{ nullptr, nullptr, true, ERPCQueueType::Send, ERPCResult::UnresolvedParameters };
}

void USchemaGenObjectStub::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USchemaGenObjectStub, IntValue);
	DOREPLIFETIME(USchemaGenObjectStub, BoolValue);
}
