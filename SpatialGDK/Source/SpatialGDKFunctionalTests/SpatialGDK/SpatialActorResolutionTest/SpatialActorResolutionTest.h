#pragma once

#include "GameFramework/Actor.h"

#include "SpatialCommonTypes.h"
#include "SpatialFunctionalTest.h"

#include "SpatialActorResolutionTest.generated.h"

UCLASS()
class ASelfDestroyingActor : public AActor
{
	GENERATED_BODY()

public:
	ASelfDestroyingActor();

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
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
