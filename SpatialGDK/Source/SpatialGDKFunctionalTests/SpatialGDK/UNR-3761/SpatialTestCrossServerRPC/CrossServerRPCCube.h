// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"

#include "CrossServerRPCCube.generated.h"

UCLASS()
class ACrossServerRPCCube : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	ACrossServerRPCCube();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void TurnOnReplication();
	void SetNonAuth();
	void RecordEntityId();

	// Array storing the IDs of the servers from which this cube has successfully received a CrossServer RPC.
	UPROPERTY(Replicated)
	TArray<int> ReceivedCrossServerRPCS;

	UPROPERTY(Replicated)
	int64 AuthEntityId;

	UFUNCTION(CrossServer, Reliable)
	void CrossServerTestRPC(int SendingServerID);
};
