// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "ReplicatedTearOffActor.generated.h"

class ASpatialTestTearOff;

/**
 * A replicated Actor that calls TearOff on BeginPlay(). Used in the SpatialTearoffMap and inside the SpatialTestTearOff class.
 */
UCLASS()
class AReplicatedTearOffActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AReplicatedTearOffActor();

	void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int TestInteger;
};
