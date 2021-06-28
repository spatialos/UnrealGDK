// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "BlueprintRepPropertyDormancyTest.h"
#include "DormancyTestActor.h"
#include "EngineUtils.h"

/**
 * This test tests whether modifying a replicated property in blueprints on a dormant actor will trigger the actor to wake up and the
 * property to correctly replicate.
 *
 * The test includes a single server and two client workers. The client workers begin with a player controller and their default pawns,
 * which they initially possess. The test also REQUIRES the presence of a BP_DormancyTestActor (this actor is initially dormant) in the
 * level where it is placed. The flow is as follows:
 *  - Setup:
 *    - (Refer to above about placing instructions).
 *  - Test:
 *    - The server modifies the dormant actor's TestIntProp in blueprints so that the actor should awake.
 *    - The client verifies that locally the value of TestIntProp has changed and it's dormancy updated.
 *  - Cleanup:
 *    - No cleanup required, as the actor is deleted as part of the test. Note that the actor exists in the world if other tests are run
 * before this one.
 *    - Note that this test cannot be rerun, as it relies on an actor placed in the level being deleted as part of the test.
 */
ABlueprintRepPropertyDormancyTest::ABlueprintRepPropertyDormancyTest()
{
	Author = "Matthew Sandford";
}

void ABlueprintRepPropertyDormancyTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Modify the TestIntProp in blueprints
	AddStep(TEXT("ServerModifyRepPropertyValueInBlueprints"), FWorkerDefinition::Server(1), nullptr, [this]() {
		int Counter = 0;
		int ExpectedDormancyActors = 1;
		for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
		{
			Counter++;
			Iter->UpdateTestIntProp();
		}
		RequireEqual_Int(Counter, ExpectedDormancyActors, TEXT("Number of TestDormancyActors in the server world"));
		FinishStep();
	});

	// Step 2 - Observe TestIntProp has been changed and the test actor dormancy has been updated on the client.
	AddStep(
		TEXT("ClientCheckDormancyAndRepPropertyUpdated"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			int Counter = 0;
			int ExpectedDormancyActors = 1;
			for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
			{
				if (Iter->NetDormancy != DORM_Initial && Iter->TestIntProp != 0)
				{
					Counter++;
				}
			}
			RequireEqual_Int(Counter, ExpectedDormancyActors, TEXT("Number of TestDormancyActors in client world"));
			FinishStep();
		},
		5.0f);
}
