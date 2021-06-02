// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "StaticSubobjectTestActor.h"

#include "StaticSubobjectsTest.generated.h"

class ATestMovementCharacter;
class ADynamicSubObjectTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API AStaticSubobjectsTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	AStaticSubobjectsTest();

	virtual void PrepareTest() override;

	void DestroyNonRootComponents() const;
	int GetNumComponentsOnTestActor();
	AStaticSubobjectTestActor* GetReplicatedTestActor();

	// A reference to the Default Pawn of Client 1 to allow for repossession in the final step of the test.
	APawn* ClientOneDefaultPawn;

	ATestMovementCharacter* ClientOneSpawnedPawn;

	AStaticSubobjectTestActor* TestActor;

	const FVector PawnSpawnLocation = FVector(0.0f, 120.0f, 40.0f);

	const FVector PawnMovedToRemoteLocation = FVector(20000.0f, 20000.0f, 40.0f);

	static constexpr int32 InitialNumComponents = 2;

	static constexpr float TimeLimit = 100.0f;

	float StepTimer;
};
