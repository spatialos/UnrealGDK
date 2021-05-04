// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "ReplicatedTestActorBase.h"
#include "ReplicatedTestActorBase_RepGraphAlwaysReplicate.generated.h"

/**
 * A replicated Actor which is always replicated by RepGraph regardless of client view.
 */
UCLASS()
class AReplicatedTestActorBase_RepGraphAlwaysReplicate : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
};
