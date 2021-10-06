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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void OnAuthorityGained() override;

	UFUNCTION(CrossServer, Reliable)
	void AcquireLock(int ServerID);

	UFUNCTION(CrossServer, Reliable)
	void ReleaseLock();

	UPROPERTY(Replicated)
	int LockingServerID;

	UPROPERTY(Replicated)
	int AuthorityChanges;

private:
	FLockingToken LockTocken;
};
