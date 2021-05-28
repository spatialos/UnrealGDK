// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"

#include "DynamicSubobjectsTest.generated.h"

namespace boost {
	namespace python {
		class override;
	}
}

class ATestMovementCharacter;
class ADynamicSubObjectTestActor;

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ADynamicSubobjectsTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ADynamicSubobjectsTest();

	virtual void PrepareTest() override;
	ADynamicSubObjectTestActor* GetReplicatedTestActor();
	int GetNumComponentsOnTestActor();

	// A reference to the Default Pawn of Client 1 to allow for repossession in the final step of the test.
	APawn* ClientOneDefaultPawn;

	ATestMovementCharacter* ClientOneSpawnedPawn;

	ADynamicSubObjectTestActor* TestActor;

	// The spawn location for Client 1's Pawn;
	FVector CharacterSpawnLocation = FVector(0.0f, 120.0f, 40.0f);

	// A remote location where Client 1's Pawn will be moved in order to not see the AReplicatedVisibilityTestActor.
	FVector CharacterRemoteLocation = FVector(20000.0f, 20000.0f, 40.0f); // Outside of the interest range of the client

	static constexpr int32 InitialNumComponents = 1;

	static constexpr float TimeLimit = 100.0f;

	float StepTimer;
};
