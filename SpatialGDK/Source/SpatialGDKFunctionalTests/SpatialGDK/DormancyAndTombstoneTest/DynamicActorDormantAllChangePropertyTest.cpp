// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicActorDormantAllChangePropertyTest.h"

#include "DormancyTestActor.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialActorChannel.h"

// The test checks that modifying a replicated property on an actor that has NetDormancy DORM_DormantAll will result in the property not
// being replicated.

ADynamicActorDormantAllChangePropertyTest::ADynamicActorDormantAllChangePropertyTest()
{
	Author = "Matthew Sandford";
}

void ADynamicActorDormantAllChangePropertyTest::PrepareTest()
{
	Super::PrepareTest();

	// Step 1 - Spawn dormancy actor and check NetDormancy is DORM_Initial
	AddStep(TEXT("ServerSpawnDormancyActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TestActor = CreateDormancyTestActor();
		TestActor->SetNetDormancy(DORM_DormantAll);
		FinishStep();
	});

	// Step 2 - Wait for actor channel to be ReadyForDormancy
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {

		bool bIsReadyForDormancy = true;

		UWorld* World = GetWorld();
		USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());

		if (SpatialNetDriver != nullptr)
		{
			for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
			{
				USpatialActorChannel* ActorChannel = SpatialNetDriver->GetOrCreateSpatialActorChannel(*Iter);
				bIsReadyForDormancy = ActorChannel->ReadyForDormancy();
			}
		}

		RequireTrue(bIsReadyForDormancy, TEXT("Ready for dormancy"));
		FinishStep();
	},
		5.0f);

	// Step 3 - Client Check NetDormancy is DORM_Initial
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			CheckDormancyAndRepProperty(DORM_DormantAll, 0);
			FinishStep();
		},
		5.0f);

	// Step 4 - Server set TestIntProp to 1
	AddStep(TEXT("ServerModifyNetDormancy"), FWorkerDefinition::Server(1), nullptr, [this]() {
		for (TActorIterator<ADormancyTestActor> Iter(GetWorld()); Iter; ++Iter)
		{
			ADormancyTestActor* DormancyTestActor = *Iter;
			DormancyTestActor->TestIntProp = 1;
		}
		FinishStep();
	});

	// Step 5 - Client Check TestIntProp is 0
	AddStep(
		TEXT("ClientCheckDormancyAndRepProperty"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			CheckDormancyAndRepProperty(DORM_DormantAll, 0);
			FinishStep();
		},
		5.0f);

	// Step 6 - Delete the test actor on the server.
	AddStep(TEXT("ServerDeleteActor"), FWorkerDefinition::Server(1), nullptr, [this]() {
		DestroyDormancyTestActors();
		FinishStep();
	});
}
