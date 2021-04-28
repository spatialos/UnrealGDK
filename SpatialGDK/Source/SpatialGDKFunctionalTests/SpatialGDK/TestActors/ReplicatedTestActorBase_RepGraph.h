// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReplicatedTestActorBase.h"
#include "ReplicatedTestActorBase_RepGraph.generated.h"

/**
 * A replicated Actor which is always replicated by RepGraph regardless of client view.
 */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AReplicatedTestActorBase_RepGraph : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AReplicatedTestActorBase_RepGraph();
};
