// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "DynamicSubobjectsTest.generated.h"

class ATestMovementCharacter;
class AReplicatedGASTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADynamicSubobjectsTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ADynamicSubobjectsTest();

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
