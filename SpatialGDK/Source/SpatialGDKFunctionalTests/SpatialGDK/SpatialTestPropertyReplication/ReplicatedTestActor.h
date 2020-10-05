// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "ReplicatedTestActor.generated.h"

UCLASS()
class AReplicatedTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AReplicatedTestActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int ExampleReplicatedProperty;
};
