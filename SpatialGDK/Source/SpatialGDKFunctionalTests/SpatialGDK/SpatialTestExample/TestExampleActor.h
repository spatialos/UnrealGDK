// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "TestExampleActor.generated.h"

UCLASS()
class ATestExampleActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	ATestExampleActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int ExampleReplicatedProperty;
};
