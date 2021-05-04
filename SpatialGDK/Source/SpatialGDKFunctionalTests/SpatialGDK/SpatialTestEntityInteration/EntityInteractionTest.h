// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestMaps/GeneratedTestMap.h"

#include "EntityInteractionTest.generated.h"

class AEntityInteractionTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialEntityInteractionTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialEntityInteractionTest();

	virtual void PrepareTest() override;

	uint32 NumSteps = 0;
	TMap<uint32, FString> ExpectedResult;
	TStaticArray<AEntityInteractionTestActor*, 2> LocalActors;
	TStaticArray<AEntityInteractionTestActor*, 2> RemoteActors;
};
