// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "VisibilityTest.generated.h"

class ATestMovementCharacter;
class AReplicatedVisibilityTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AVisibilityTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	AVisibilityTest();

	virtual void BeginPlay() override;

	int GetNumberOfVisibilityTestActors();

	// A reference to the Default Pawn of Client 1 to allow for repossession in the final step of the test.
	APawn* ClientOneDefaultPawn;

	ATestMovementCharacter* ClientOneSpawnedPawn;

	AReplicatedVisibilityTestActor* TestActor;

	// The spawn location for Client 1's Pawn;
	FVector CharacterSpawnLocation;

	// A remote location where Client 1's Pawn will be moved in order to not see the AReplicatedVisibilityTestActor.
	FVector CharacterRemoteLocation;
};
