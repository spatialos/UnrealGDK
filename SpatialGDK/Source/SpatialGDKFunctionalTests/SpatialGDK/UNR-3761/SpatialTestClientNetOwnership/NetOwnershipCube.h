// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"

#include "CoreMinimal.h"

#include "NetOwnershipCube.generated.h"

UCLASS()
class ANetOwnershipCube : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	ANetOwnershipCube();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerIncreaseRPCCount();

	UPROPERTY(Replicated)
	int ReceivedRPCs;
};
