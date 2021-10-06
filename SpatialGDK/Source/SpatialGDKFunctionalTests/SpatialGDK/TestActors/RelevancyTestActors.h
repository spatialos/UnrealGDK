// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ReplicatedTestActorBase.h"
#include "RelevancyTestActors.generated.h"

/**
 * An always relevant, replicated Actor. NCD set small to ensure AlwaysRelevant used
 */
UCLASS()
class AAlwaysRelevantTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AAlwaysRelevantTestActor();
};

/**
 * An always relevant, server only, replicated Actor.
 */
UCLASS(SpatialType = ServerOnly)
class AAlwaysRelevantServerOnlyTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AAlwaysRelevantServerOnlyTestActor();
};

/**
 * An only relevant to owner, replicated Actor.
 */
UCLASS(SpatialType = ServerOnly)
class AOnlyRelevantToOwnerTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AOnlyRelevantToOwnerTestActor();
};

/**
 * A use owner relevancy, replicated Actor.
 */
UCLASS(SpatialType = ServerOnly)
class AUseOwnerRelevancyTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AUseOwnerRelevancyTestActor();
};
