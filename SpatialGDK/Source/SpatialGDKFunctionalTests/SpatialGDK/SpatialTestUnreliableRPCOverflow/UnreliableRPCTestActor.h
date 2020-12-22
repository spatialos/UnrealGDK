// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "UnreliableRPCTestActor.generated.h"

UCLASS()
class AUnreliableRPCTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AUnreliableRPCTestActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void LaunchRPCs(int RPCCount);

	UFUNCTION(Client, Unreliable)
	void TestRPC(const int Value);

	const TArray<int>& GetArray() const;

private:
	UPROPERTY(Replicated)
	TArray<int> TestArray;
};
