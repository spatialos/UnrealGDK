// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicSubobjectsTest.h"
#include "ReplicatedGASTestActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "SpatialGDKSettings.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "Components/SceneComponent.h"

#include "EngineClasses/SpatialNetDriver.h"

/**
 * Tests if the dynamic sub-object of the AReplicatedGASTestActor is not duplicated on Clients when leaving
 * and re-entering interest.
 *
 * The test includes a single server and one client worker.
 * The flow is as follows:
 *  - Setup:
 *	  - One cube actor already placed in the level at Location FVector(0.0f, 0.0f, 80.0f) needs to be a startup actor - bNetLoadOnClient =
 *true.
 *    - The Server spawns a TestMovementCharacter and makes Client 1 possess it.
 *  - Test:
 *    - Each worker tests if it can initially see the AReplicatedGASTestActor.
 *    - Repeat the following steps MaxDynamicallyAttachedSubobjectsPerClass + 1 times:
 *		- After ensuring possession happened, the Server moves Client 1's Character to a remote location, so it cannot see the
 *AReplicatedGASTestActor.
 *		- After ensuring movement replicated correctly, Client 1 checks it can no longer see the AReplicatedGASTestActor.
 *		- The Server moves the character of Client 1 back close to its spawn location, so that the AReplicatedGASTestActor is
 *in its interest area.
 *	  - If the "Too many dynamic sub objects" error does not appears in the log the test is successful.
 *  - Cleanup:
 *    - Client 1 repossesses its default pawn.
 *    - The spawned Character is destroyed.
 *
 *
 * A second test case is also tested with this same test.
 * This tests that
 * 1. The server adds a dynamic component to the actor
 * 1. AReplicatedGASTestActor moves out of the client's interest
 * 2. AReplicatedGASTestActor has the dynamic component removed
 * 3. AReplicatedGASTestActor moves into the client's interest
 * 4. The client sees AReplicatedGASTestActor no longer has the dynamic component
 *
 * This extra test case is implemented in steps 9.1 and 12.1
 */

const static float StepTimeLimit = 15.0f;

ADynamicSubobjectsTest::ADynamicSubobjectsTest()
	: Super()
{
	Author = "Evi&Arthur&Miron";
	Description = TEXT("Test Dynamic Subobjects Duplication in Client");

	CharacterSpawnLocation = FVector(0.0f, 120.0f, 40.0f);
	CharacterRemoteLocation = FVector(20000.0f, 20000.0f, 40.0f); // Outside of the interest range of the client

	TimeLimit = 100.0f;
}

void ADynamicSubobjectsTest::PrepareTest()
{
	Super::PrepareTest();

	const int DynamicComponentsPerClass = GetDefault<USpatialGDKSettings>()->MaxDynamicallyAttachedSubobjectsPerClass;
	StepTimer = 0.0f;

	// Step 0 - The server spawn a TestMovementCharacter and makes Client 1 possess it.
	AddStep(TEXT("DynamicSubobjectsTestSetup"), FWorkerDefinition::Server(1), nullptr, [this]() {
		ASpatialFunctionalTestFlowController* ClientOneFlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		APlayerController* PlayerController = Cast<APlayerController>(ClientOneFlowController->GetOwner());

		if (AssertIsValid(PlayerController, TEXT("PlayerController should be valid")))
		{
			ClientOneSpawnedPawn = GetWorld()->SpawnActor<ATestMovementCharacter>(CharacterSpawnLocation, FRotator::ZeroRotator);
			RegisterAutoDestroyActor(ClientOneSpawnedPawn);

			ClientOneDefaultPawn = PlayerController->GetPawn();
			PlayerController->Possess(ClientOneSpawnedPawn);

			FinishStep();
		}
	});

	// Step 1 - All workers check if they have one AReplicatedGASTestActor in the world, and set a reference to it.
	AddStep(
		TEXT("DynamicSubobjectsTestAllWorkers"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float DeltaTime) {
			TestActor = GetReplicatedTestActor();
			FinishStep();
		},
		StepTimeLimit);

	// Step 2 - Client 1 checks if it has correctly possessed the TestMovementCharacter.
	AddStep(
		TEXT("DynamicSubobjectsTestClientCheckPossesion"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float DeltaTime) {
			APawn* PlayerCharacter = GetFlowPawn();
			if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should be valid")))
			{
				RequireTrue(PlayerCharacter == GetFlowPlayerController()->AcknowledgedPawn, TEXT("The client should possess the pawn."));
				FinishStep();
			}
		},
		StepTimeLimit);

	// Step 3 - The client checks it has the right initial amount of components
	AddStep(TEXT("DynamicSubobjectsTestClientCheckNumComponents"), FWorkerDefinition::Client(1), nullptr, [this]() {
		AssertEqual_Int(GetNumComponentsOnTestActor(), 3, TEXT("For this test, DynamicSubobjectTestActor should have 3 components"));
		FinishStep();
	});

	// Step 4 - The server adds the new dynamic component
	AddStep(TEXT("DynamicSubobjectsTestServerAddComponent"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AssertEqual_Int(GetNumComponentsOnTestActor(), 3, TEXT("For this test, DynamicSubobjectTestActor should have 3 components"));

		// add new dynamic component to test actor
		USceneComponent* AddedComponent = NewObject<USceneComponent>(TestActor, TEXT("ToRemoveComponent"));
		AddedComponent->AttachToComponent(TestActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		AddedComponent->RegisterComponent();
		AddedComponent->SetIsReplicated(true);

		AssertEqual_Int(GetNumComponentsOnTestActor(), 4, TEXT("Now DynamicSubobjectTestActor should have 4 components"));
		FinishStep();
	});

	// Step 5 - The client waits till it can see the new component
	AddStep(
		TEXT("DynamicSubobjectsTestClientSeeNewComponent"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(GetNumComponentsOnTestActor(), 4, TEXT("Now DynamicSubobjectTestActor should have 4 components"));
			FinishStep();
		},
		StepTimeLimit);

	for (int i = 0; i < DynamicComponentsPerClass + 2; ++i)
	{
		const bool bLastStepLoop = i == DynamicComponentsPerClass + 1;

		// Step 6 - Server moves the TestMovementCharacter of Client 1 to a remote location, so that it does not see the
		// AReplicatedGASTestActor.
		AddStep(TEXT("DynamicSubobjectsTestServerMoveClient1"), FWorkerDefinition::Server(1), nullptr, [this]() {
			ClientOneSpawnedPawn->SetActorLocation(CharacterRemoteLocation);
			AssertEqual_Vector(ClientOneSpawnedPawn->GetActorLocation(), CharacterRemoteLocation,
							   TEXT("Client pawn was not moved to remote location"), 1.0f);
			FinishStep();
		});

		// Step 7 - Client 1 makes sure that the movement was correctly replicated
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckFirstMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				APawn* PlayerCharacter = GetFlowPawn();

				if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should not be nullptr")))
				{
					RequireEqual_Vector(PlayerCharacter->GetActorLocation(), CharacterRemoteLocation,
										TEXT("Character was not moved to remote location"), 1.0f);
					FinishStep();
				}
			},
			StepTimeLimit);

		// When in native, we need to wait for a while here - so the engine can update relevancy
		const bool bIsSpatial = Cast<USpatialNetDriver>(GetNetDriver()) != nullptr;
		if (!bIsSpatial)
		{
			AddStep(
				TEXT("DynamicSubobjectsTestNativeWaitABit"), FWorkerDefinition::Server(1), nullptr,
				[this]() {
					StepTimer = 0.f;
				},
				[this](float DeltaTime) {
					StepTimer += DeltaTime;
					if (StepTimer > 7.5f)
					{
						FinishStep();
					}
				});
		}

		// Step 8 - Server increases AReplicatedGASTestActor's TestIntProperty to enable checking if the client is out of interest later.
		AddStep(TEXT("DynamicSubobjectsTestServerIncreasesIntValue"), FWorkerDefinition::Server(1), nullptr, [this, i]() {
			TestActor->TestIntProperty = i;
			FinishStep();
		});

		// Step 9 - Client 1 checks it can no longer see the AReplicatedGASTestActor by waiting for 0.5s and checking TestIntProperty hasn't
		// updated
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased"), FWorkerDefinition::Client(1), nullptr,
			[this]() {
				StepTimer = 0.f;
			},
			[this, i](float DeltaTime) {
				RequireNotEqual_Int(TestActor->TestIntProperty, i, TEXT("Check TestIntProperty didn't get replicated"));
				StepTimer += DeltaTime;
				if (StepTimer >= 0.5f)
				{
					FinishStep();
				}
			},
			StepTimeLimit);

		if (bLastStepLoop)
		{
			// Step 9.1 - Server removes the component for secondary test case
			AddStep(TEXT("DynamicSubobjectsTestServerDestroyActorComponent"), FWorkerDefinition::Server(1), nullptr, [this]() {
				TArray<USceneComponent*> AllSceneComps;
				TestActor->GetComponents<USceneComponent>(AllSceneComps);
				AssertEqual_Int(AllSceneComps.Num(), 4, TEXT("DynamicSubobjectTestActor should have 4 components"));

				// Delete the component with the right name
				for (USceneComponent* SceneComponent : AllSceneComps)
				{
					if (SceneComponent->GetName() == TEXT("ToRemoveComponent"))
					{
						SceneComponent->DestroyComponent();
					}
				}

				AssertEqual_Int(GetNumComponentsOnTestActor(), 3, TEXT("Now DynamicSubobjectTestActor should have 3 components"));
				FinishStep();
			});
		}

		// Step 10 - Server moves Client 1 close to the cube.
		AddStep(TEXT("DynamicSubobjectsTestServerMoveClient1CloseToCube"), FWorkerDefinition::Server(1), nullptr, [this]() {
			ClientOneSpawnedPawn->SetActorLocation(CharacterSpawnLocation);
			AssertEqual_Vector(ClientOneSpawnedPawn->GetActorLocation(), CharacterSpawnLocation,
							   TEXT("Server 1 should see the pawn close to the initial spawn location"), 1.0f);
			FinishStep();
		});

		// Step 11 - Client 1 checks that the movement was replicated correctly.
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckSecondMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				APawn* PlayerCharacter = GetFlowPawn();

				if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should be valid")))
				{
					RequireEqual_Vector(PlayerCharacter->GetActorLocation(), CharacterSpawnLocation,
										TEXT("Client 1 should see themself close to the initial spawn location"), 1.0f);
					FinishStep();
				}
			},
			StepTimeLimit);

		// Step 12 - Client 1 checks it can see the AReplicatedGASTestActor
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased2"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this, i](float DeltaTime) {
				RequireEqual_Int(TestActor->TestIntProperty, i, TEXT("Client 1 should see the updated TestIntProperty value"));
				FinishStep();
			},
			StepTimeLimit);

		if (bLastStepLoop)
		{
			// Step 12.1 - Client 1 checks the dynamic component on ReplicatedGASTestActor has been removed
			AddStep(
				TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased2"), FWorkerDefinition::Client(1), nullptr, nullptr,
				[this](float DeltaTime) {
					RequireEqual_Int(GetNumComponentsOnTestActor(), 3,
									 TEXT("DynamicSubobjectTestActor's dynamic component should have been destroyed."));

					FinishStep();
				},
				StepTimeLimit);
		}
	}

	// Step 13 - All worker cleanup
	AddStep(TEXT("DynamicSubobjectsTestAllWorkerCleanup"), FWorkerDefinition::AllWorkers, nullptr, [this]() {
		TestActor->TestIntProperty = -1;
		FinishStep();
	});

	// Step 14 - Server Cleanup.
	AddStep(TEXT("DynamicSubobjectsTestServerCleanup"), FWorkerDefinition::Server(1), nullptr, [this]() {
		// Possess the original pawn, so that the spawned character can get destroyed correctly
		ASpatialFunctionalTestFlowController* ClientOneFlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		APlayerController* PlayerController = Cast<APlayerController>(ClientOneFlowController->GetOwner());

		if (AssertIsValid(PlayerController, TEXT("PlayerController should be valid")))
		{
			PlayerController->Possess(ClientOneDefaultPawn);
			FinishStep();
		}
	});
}

AReplicatedGASTestActor* ADynamicSubobjectsTest::GetReplicatedTestActor()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedGASTestActor::StaticClass(), FoundActors);
	if (AssertEqual_Int(FoundActors.Num(), 1, TEXT("There should only be one actor of type AReplicatedGASTestActor in the world")))
	{
		TestActor = Cast<AReplicatedGASTestActor>(FoundActors[0]);
		if (AssertIsValid(TestActor, TEXT("TestActor must be valid")))
		{
			return TestActor;
		}
	}
	return nullptr;
}

int ADynamicSubobjectsTest::GetNumComponentsOnTestActor()
{
	TestActor = GetReplicatedTestActor();
	TArray<USceneComponent*> AllActorComp;
	TestActor->GetComponents<USceneComponent>(AllActorComp);

	return AllActorComp.Num();
}
