// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "RefreshActorDormancyTestActor.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ARefreshActorDormancyTestActor : public AReplicatedTestActorBase
{
	GENERATED_BODY()

public:
	void FlushDormancy();
	void SetInitiallyDormant(bool bInitiallyDormant);
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

private:
	bool bFirstRep = true;
	bool bIsSetDormancyComplete = false;
	bool bSetupDormancyStateForTest = false;
	ENetDormancy FinalDormancyState;
	FDelegateHandle PostTickFlushDelegateHandle;
};
