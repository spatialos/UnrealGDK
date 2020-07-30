// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "ReplicatedVisibilityTestActor.generated.h"

UCLASS()
class AReplicatedVisibilityTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AReplicatedVisibilityTestActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
