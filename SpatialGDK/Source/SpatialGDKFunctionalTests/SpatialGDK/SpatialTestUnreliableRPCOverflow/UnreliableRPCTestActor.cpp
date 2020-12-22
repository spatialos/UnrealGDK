// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "UnreliableRPCTestActor.h"
#include "Net/UnrealNetwork.h"

AUnreliableRPCTestActor::AUnreliableRPCTestActor() {}

void AUnreliableRPCTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUnreliableRPCTestActor, TestArray);
}

void AUnreliableRPCTestActor::LaunchRPCs(int RPCCount)
{
	for (auto i = 0; i < RPCCount; ++i)
	{
		TestRPC(i);
	}
}

void AUnreliableRPCTestActor::TestRPC_Implementation(const int Value)
{
	TestArray.Add(Value);
}

const TArray<int>& AUnreliableRPCTestActor::GetArray() const
{
	return TestArray;
}
