// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ReplicatedCrossServerRPCCube.h"
#include "NonReplicatedCrossServerRPCCube.generated.h"

UCLASS()
class ANonReplicatedCrossServerRPCCube : public AReplicatedCrossServerRPCCube
{
	GENERATED_BODY()

public:
	ANonReplicatedCrossServerRPCCube();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void TurnOnReplication();
	void SetNonAuth();
};
