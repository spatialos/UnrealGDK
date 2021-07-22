
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestPropertyReplicationSubobject.generated.h"

class AReplicatedTestActorSubobject;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialTestPropertyReplicationSubobject : public ASpatialFunctionalTest
{
	GENERATED_BODY()
public:
	ASpatialTestPropertyReplicationSubobject();

	virtual void PrepareTest() override;

private:
	UPROPERTY()
	AReplicatedTestActorSubobject* TestActor;
};
