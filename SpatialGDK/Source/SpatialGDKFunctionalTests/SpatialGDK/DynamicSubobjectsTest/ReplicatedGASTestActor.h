// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"

#include "ReplicatedGASTestActor.generated.h"

UCLASS()
class AReplicatedGASTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AReplicatedGASTestActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int TestIntProperty;
};
