// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/DefaultPawn.h"
#include "ReplicatedTestPawnBase_RepGraphAlwaysReplicate.generated.h"

/**
 * A replicated Pawn which is always replicated by RepGraph regardless of client view.
 */
UCLASS()
class AReplicatedTestPawnBase_RepGraphAlwaysReplicate : public ADefaultPawn
{
	GENERATED_BODY()

public:
	AReplicatedTestPawnBase_RepGraphAlwaysReplicate();

};
