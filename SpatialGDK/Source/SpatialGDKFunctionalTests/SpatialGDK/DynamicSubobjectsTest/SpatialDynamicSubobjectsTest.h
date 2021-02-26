// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "TestMaps/GeneratedTestMap.h"
#include "SpatialDynamicSubobjectsTest.generated.h"

class ATestMovementCharacter;
class AReplicatedGASTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialDynamicSubobjectsTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialDynamicSubobjectsTest();

	virtual void PrepareTest() override;

	// A reference to the Default Pawn of Client 1 to allow for repossession in the final step of the test.
	APawn* ClientOneDefaultPawn;

	ATestMovementCharacter* ClientOneSpawnedPawn;

	AReplicatedGASTestActor* TestActor;

	// The spawn location for Client 1's Pawn;
	FVector CharacterSpawnLocation;

	// A remote location where Client 1's Pawn will be moved in order to not see the AReplicatedVisibilityTestActor.
	FVector CharacterRemoteLocation;

	float StepTimer;
};

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API USpatialDynamicSubobjectsMap : public UGeneratedTestMap
{
	GENERATED_BODY()

public:
	USpatialDynamicSubobjectsMap();

protected:
	virtual void CreateCustomContentForMap() override;
};
