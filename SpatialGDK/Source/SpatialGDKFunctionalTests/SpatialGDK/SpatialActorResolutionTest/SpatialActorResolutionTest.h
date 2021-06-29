#pragma once

#include "SpatialCommonTypes.h"
#include "SpatialFunctionalTest.h"

#include "SpatialActorResolutionTest.generated.h"

UCLASS()
class ASelfDestroyingActor : public AActor
{
	GENERATED_BODY()

public:
	ASelfDestroyingActor();

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
};

UCLASS()
class ASpatialActorResolutionTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialActorResolutionTest();

	virtual void PrepareTest() override;

	UPROPERTY(Transient)
	TWeakObjectPtr<ASelfDestroyingActor> TestActor;

	float TimeElapsed;
};
