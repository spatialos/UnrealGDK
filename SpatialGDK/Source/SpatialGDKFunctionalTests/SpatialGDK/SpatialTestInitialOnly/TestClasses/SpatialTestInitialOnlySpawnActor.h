// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "SpatialTestInitialOnlySpawnComponent.h"
#include "SpatialTestInitialOnlySpawnActor.generated.h"

class USpatialTestInitialOnlySpawnComponent;

UCLASS()
class ASpatialTestInitialOnlySpawnActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	ASpatialTestInitialOnlySpawnActor();

	UPROPERTY(Replicated)
	int Int_Initial = 1;

	UPROPERTY(Replicated)
	int Int_Replicate = 1;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
