// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"

#include "PartiallyStablePathActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class APartiallyStablePathActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	APartiallyStablePathActor() {}

	virtual void BeginPlay() override;

	UPROPERTY()
	UStaticMeshComponent* DynamicComponent;
};
