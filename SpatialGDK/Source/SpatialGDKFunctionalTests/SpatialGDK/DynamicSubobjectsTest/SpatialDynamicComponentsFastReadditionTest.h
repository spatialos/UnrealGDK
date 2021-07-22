#pragma once

#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"

#include "SpatialFunctionalTest.h"

#include "SpatialDynamicComponentsFastReadditionTest.generated.h"

UCLASS()
class ADynamicComponentsTestActor : public AActor
{
public:
	GENERATED_BODY()

	ADynamicComponentsTestActor();
};

UCLASS()
class USelfRecreatingDynamicComponent : public UActorComponent
{
public:
	GENERATED_BODY()

	USelfRecreatingDynamicComponent();

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

private:
	bool bNeedsToRecreateItself;
};

UCLASS()
class ASpatialDynamicComponentsFastReadditionTest : public ASpatialFunctionalTest
{
public:
	GENERATED_BODY()

	ASpatialDynamicComponentsFastReadditionTest();

	virtual void PrepareTest() override;

private:
	ADynamicComponentsTestActor* FindTestActor();
	float StepTimeCounter = 0.0f;
};
