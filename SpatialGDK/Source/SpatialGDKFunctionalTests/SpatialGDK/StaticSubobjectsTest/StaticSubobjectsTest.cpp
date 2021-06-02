// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

// #pragma optimize("", off)

#include "StaticSubobjectsTest.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "StaticSubObjectTestActor.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "Components/SceneComponent.h"

#include "EngineClasses/SpatialNetDriver.h"

/**
 * Tests if the dynamic sub-object of the AStaticSubobjectTestActor is not duplicated on Clients when leaving
 * and re-entering interest.
 *
 * The test includes a single server and one client worker.
 * The flow is as follows:
 *  - Setup:
 *	  - One cube actor already placed in the level at Location FVector(0.0f, 0.0f, 80.0f) needs to be a startup actor - bNetLoadOnClient =
 *true.
 *    - The Server spawns a TestMovementCharacter and makes Client 1 possess it.
 *  - Test:
 *    - Each worker tests if it can initially see the AStaticSubobjectTestActor.
 *    - Repeat the following steps MaxDynamicallyAttachedSubobjectsPerClass + 1 times:
 *		- After ensuring possession happened, the Server moves Client 1's Character to a remote location, so it cannot see the
 *AStaticSubobjectTestActor.
 *		- After ensuring movement replicated correctly, Client 1 checks it can no longer see the AStaticSubobjectTestActor.
 *		- The Server moves the character of Client 1 back close to its spawn location, so that the AStaticSubobjectTestActor is
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
 * 1. AStaticSubobjectTestActor moves out of the client's interest
 * 2. AStaticSubobjectTestActor has the dynamic component removed
 * 3. AStaticSubobjectTestActor moves into the client's interest
 * 4. The client sees AStaticSubobjectTestActor no longer has the dynamic component
 *
 * This extra test case is implemented in steps 9.1 and 12.1
 */

static constexpr float StepTimeLimit = 15.0f;

AStaticSubobjectsTest::AStaticSubobjectsTest()
	: Super()
{
	Author = "Evi&Arthur&Miron";
	Description = TEXT("Test Dynamic Subobjects Duplication in Client");
}

void AStaticSubobjectsTest::PrepareTest()
{
	Super::PrepareTest();

	StepTimer = 0.0f;

	// Step 0 - The server spawn a TestMovementCharacter and makes Client 1 possess it.
	AddStep(TEXT("DynamicSubobjectsTestSetup"), FWorkerDefinition::Server(1), nullptr, [this]() {
		ASpatialFunctionalTestFlowController* ClientOneFlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		APlayerController* PlayerController = Cast<APlayerController>(ClientOneFlowController->GetOwner());

		if (AssertIsValid(PlayerController, TEXT("PlayerController should be valid")))
		{
			ClientOneSpawnedPawn = GetWorld()->SpawnActor<ATestMovementCharacter>(PawnSpawnLocation, FRotator::ZeroRotator);
			RegisterAutoDestroyActor(ClientOneSpawnedPawn);

			ClientOneDefaultPawn = PlayerController->GetPawn();
			PlayerController->Possess(ClientOneSpawnedPawn);

			FinishStep();
		}
	});

	// Step 1 - All workers check for one AStaticSubobjectTestActor in the world and initialise it.
	AddStep(
		TEXT("StaticSubobjectsTestAllWorkers"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float DeltaTime) {
			TestActor = GetReplicatedTestActor();
			TestActor->InitialiseTestIntProperty();
			FinishStep();
		},
		StepTimeLimit);

	// Step 2 - Client 1 checks if it has correctly possessed the TestMovementCharacter.
	AddStep(
		TEXT("StaticSubobjectsTestClientCheckPossesion"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float DeltaTime) {
			APawn* PlayerCharacter = GetFlowPawn();
			if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should be valid")))
			{
				RequireTrue(PlayerCharacter == GetFlowPlayerController()->AcknowledgedPawn, TEXT("The client should possess the pawn."));
				FinishStep();
			}
		},
		StepTimeLimit);

	// Step 3 - The client checks the actor has the right initial amount of components
	AddStep(TEXT("StaticSubobjectsTestClientCheckNumComponents"), FWorkerDefinition::Client(1), nullptr, [this]() {
		AssertEqual_Int(GetNumComponentsOnTestActor(), InitialNumComponents,
						TEXT("AStaticSubobjectTestActor should have the initial number of components"));
		FinishStep();
	});

	// Step 6 - Server moves the TestMovementCharacter of Client 1 to a remote location, so that it does not see the
	// AStaticSubobjectTestActor.
	AddStep(TEXT("StaticSubobjectsTestServerMoveClient1"), FWorkerDefinition::Server(1), nullptr, [this]() {
		ClientOneSpawnedPawn->SetActorLocation(PawnMovedToRemoteLocation);
		AssertEqual_Vector(ClientOneSpawnedPawn->GetActorLocation(), PawnMovedToRemoteLocation,
						   TEXT("Client pawn was not moved to remote location"), 1.0f);
		FinishStep();
	});

	// Step 7 - Client 1 makes sure that the movement was correctly replicated
	AddStep(
		TEXT("StaticSubobjectsTestClientCheckFirstMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float DeltaTime) {
			APawn* PlayerCharacter = GetFlowPawn();

			if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should not be nullptr")))
			{
				RequireEqual_Vector(PlayerCharacter->GetActorLocation(), PawnMovedToRemoteLocation,
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
			TEXT("StaticSubobjectsTestNativeWaitABit1"), FWorkerDefinition::Server(1), nullptr,
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

	// Step 8 - Server increases AStaticSubobjectTestActor's TestIntProperty to enable checking if the client is out of interest later.
	AddStep(TEXT("DynamicSubobjectsTestServerIncreasesIntValue"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TestActor->TestIntProperty = 1;
		FinishStep();
	});

	// Step 9 - Client 1 checks it can no longer see the AStaticSubobjectTestActor by waiting for 0.5s and checking TestIntProperty
	// hasn't updated
	AddStep(
		TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased"), FWorkerDefinition::Client(1), nullptr,
		[this]() {
			StepTimer = 0.f;
		},
		[this](float DeltaTime) {
			RequireNotEqual_Int(TestActor->TestIntProperty, 1, TEXT("Check TestIntProperty didn't get replicated"));
			StepTimer += DeltaTime;
			if (StepTimer >= 0.5f)
			{
				FinishStep();
			}
		},
		StepTimeLimit);

	// Step 9.1 - Server removes the static component
	AddStep(TEXT("DynamicSubobjectsTestServerDestroyActorComponent"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AssertEqual_Int(GetNumComponentsOnTestActor(), InitialNumComponents,
						TEXT("AStaticSubobjectTestActor should have the initial number of components"));

		DestroyNonRootComponents();

		AssertEqual_Int(GetNumComponentsOnTestActor(), 1,
						TEXT("All components on AStaticSubobjectTestActor, apart from the root component, should have been removed"));
		FinishStep();
	});

	// Step 10 - Server moves Client 1 close to the cube.
	AddStep(TEXT("DynamicSubobjectsTestServerMoveClient1CloseToCube"), FWorkerDefinition::Server(1), nullptr, [this]() {
		ClientOneSpawnedPawn->SetActorLocation(PawnSpawnLocation);
		AssertEqual_Vector(ClientOneSpawnedPawn->GetActorLocation(), PawnSpawnLocation,
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
				RequireEqual_Vector(PlayerCharacter->GetActorLocation(), PawnSpawnLocation,
									TEXT("Client 1 should see themself close to the initial spawn location"), 1.0f);
				FinishStep();
			}
		},
		StepTimeLimit);

	// Step 12 - Client 1 checks it can see the AStaticSubobjectTestActor
	AddStep(
		TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased2"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(TestActor->TestIntProperty, 1, TEXT("Client 1 should see the updated TestIntProperty value"));
			FinishStep();
		},
		StepTimeLimit);

	// Step 12.1 - Client 1 checks the component on ReplicatedGASTestActor has been removed
	AddStep(
		TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased2"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(GetNumComponentsOnTestActor(), 1,
							 TEXT("All components on AStaticSubobjectTestActor, apart from the root component, should have been removed."));

			FinishStep();
		},
		StepTimeLimit);

	// Step 13 - Server Cleanup.
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

AStaticSubobjectTestActor* AStaticSubobjectsTest::GetReplicatedTestActor()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticSubobjectTestActor::StaticClass(), FoundActors);
	if (AssertEqual_Int(FoundActors.Num(), 1, TEXT("There should only be one actor of type AStaticSubobjectTestActor in the world")))
	{
		TestActor = Cast<AStaticSubobjectTestActor>(FoundActors[0]);
		if (AssertIsValid(TestActor, TEXT("TestActor must be valid")))
		{
			return TestActor;
		}
	}
	return nullptr;
}

int AStaticSubobjectsTest::GetNumComponentsOnTestActor()
{
	TestActor = GetReplicatedTestActor();
	TArray<USceneComponent*> AllActorComp;
	TestActor->GetComponents<USceneComponent>(AllActorComp);

	return AllActorComp.Num();
}

void AStaticSubobjectsTest::DestroyNonRootComponents() const
{
	TArray<USceneComponent*> AllSceneComps;
	TestActor->GetComponents<USceneComponent>(AllSceneComps);

	const uint32 RootComponentId = TestActor->GetRootComponent()->GetUniqueID();
	for (USceneComponent* SceneComponent : AllSceneComps)
	{
		if (SceneComponent->GetUniqueID() != RootComponentId)
		{
			SceneComponent->DestroyComponent();
		}
	}
}
