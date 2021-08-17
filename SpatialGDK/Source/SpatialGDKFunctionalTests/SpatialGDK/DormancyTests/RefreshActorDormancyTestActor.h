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
	ARefreshActorDormancyTestActor();
	void FlushDormancy();
	void SetupForTest(bool bGoToAwakeState);
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

private:
	bool bfirstRep = true;
	bool bIsSetDormancyComplete = false;
	bool bSetupForTest = false;
	ENetDormancy FinalDormancyState;
	FDelegateHandle PostTickFlushDelegateHandle;
};
