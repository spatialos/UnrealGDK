// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "Utils/SpatialStatics.h"
#include "HandoverCube.generated.h"


UCLASS()
class AHandoverCube : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	AHandoverCube();

	UFUNCTION(Server, Reliable)
	void AcquireLock();

	UFUNCTION(Server, Reliable)
	void ReleaseLock();

private:
	FLockingToken LockTocken;
};
