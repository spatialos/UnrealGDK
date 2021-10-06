// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CrossServerRPCCube.h"
#include "NonReplicatedCrossServerRPCCube.generated.h"

UCLASS()
class ANonReplicatedCrossServerRPCCube : public ACrossServerRPCCube
{
	GENERATED_BODY()

public:
	ANonReplicatedCrossServerRPCCube();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void TurnOnReplication();
	void SetNonAuth();
};
