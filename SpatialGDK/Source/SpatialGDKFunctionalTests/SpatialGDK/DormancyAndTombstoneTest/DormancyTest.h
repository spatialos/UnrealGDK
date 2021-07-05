// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "DormancyTest.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADormancyTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

protected:

	AActor* CreateDormancyTestActor();
	void CheckDormancyAndRepProperty(const TEnumAsByte<enum ENetDormancy> ExpectedNetDormancy, const int ExpectedTestIntProp);
	void DestroyDormancyTestActors();
	void CheckDormancyActorCount(const int ExpectedCount);
};
