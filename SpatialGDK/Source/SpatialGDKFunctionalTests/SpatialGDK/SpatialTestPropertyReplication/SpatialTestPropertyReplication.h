
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestPropertyReplication.generated.h"

class AReplicatedTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestPropertyReplication : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestPropertyReplication();

	virtual void PrepareTest() override;

private:
	AReplicatedTestActor* TestActor;
};
