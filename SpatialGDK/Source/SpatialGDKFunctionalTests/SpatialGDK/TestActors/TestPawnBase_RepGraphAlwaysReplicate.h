// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/DefaultPawn.h"
#include "TestPawnBase_RepGraphAlwaysReplicate.generated.h"

/**
 * A Pawn which is always replicated by RepGraph regardless of client view.
 */
UCLASS()
class ATestPawnBase_RepGraphAlwaysReplicate : public ADefaultPawn
{
	GENERATED_BODY()

public:
	ATestPawnBase_RepGraphAlwaysReplicate();
};
