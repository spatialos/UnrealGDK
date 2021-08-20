// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ReplicatedTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "ReplicatedTestActorSubobject.generated.h"

class AReplicatedTestActor;

UCLASS()
class AReplicatedTestActorSubobject : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AReplicatedTestActorSubobject();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void OnAuthorityGained() override;

	// Subobjects
	UPROPERTY(Replicated)
	AReplicatedTestActor* ReplicatedSubActor;
};
