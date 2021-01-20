// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "ReplicatedCrossServerRPCCube.generated.h"

UCLASS()
class AReplicatedCrossServerRPCCube : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AReplicatedCrossServerRPCCube();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void RecordEntityId();

	// Array storing the IDs of the servers from which this cube has successfully received a CrossServer RPC.
	UPROPERTY(Replicated)
	TArray<int> ReceivedCrossServerRPCS;

	UPROPERTY(Replicated)
	int64 AuthEntityId;

	UFUNCTION(CrossServer, Reliable)
	void CrossServerTestRPC(int SendingServerID);
};
