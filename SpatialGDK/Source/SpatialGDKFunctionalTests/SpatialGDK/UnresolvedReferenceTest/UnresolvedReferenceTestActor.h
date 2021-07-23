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

	int RepNotifyCount;

	UPROPERTY(ReplicatedUsing = OnRep_ActorRefs)
	TArray<AReplicatedTestActorBase*> ActorRefs;

	UFUNCTION()
	void OnRep_ActorRefs();
};
