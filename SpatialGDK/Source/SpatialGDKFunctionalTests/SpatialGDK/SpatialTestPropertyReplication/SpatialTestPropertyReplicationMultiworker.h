
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestPropertyReplicationMultiworker.generated.h"

class AReplicatedTestActorBasic;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestPropertyReplicationMultiworker : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestPropertyReplicationMultiworker();

	virtual void PrepareTest() override;

private:
	UPROPERTY()
	AReplicatedTestActorBasic* TestActor;
};
