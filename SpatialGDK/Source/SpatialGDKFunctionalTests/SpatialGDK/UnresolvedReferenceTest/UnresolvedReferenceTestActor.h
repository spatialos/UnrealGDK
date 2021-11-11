// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "UnresolvedReferenceTestActor.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AUnresolvedReferenceTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	TArray<AReplicatedTestActorBase*> ActorRefs;
};
