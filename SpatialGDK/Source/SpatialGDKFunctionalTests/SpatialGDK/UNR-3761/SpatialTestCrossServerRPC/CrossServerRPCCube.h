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

	// Array storing the IDs of the servers from which this cube has successfully received a CrossServer RPC.
	UPROPERTY(Replicated)
	TArray<int> ReceivedCrossServerRPCS;

	UFUNCTION(CrossServer, Reliable)
	void CrossServerTestRPC(AActor* Sender, int SendingServerID);
};
