// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ReplicatedTestActorBase.h"
#include "hardwarebp.h"
#include "InitiallyDormantTestActor.generated.h"

/**
 * An initially dormant, replicated Actor.
 */
UCLASS()
class AInitiallyDormantTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AInitiallyDormantTestActor();

	HardwareBreakpoint HardWareBp;
};
