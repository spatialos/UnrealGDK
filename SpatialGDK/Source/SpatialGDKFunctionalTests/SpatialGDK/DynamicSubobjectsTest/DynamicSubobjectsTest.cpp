// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DynamicSubobjectsTest.h"
#include "ReplicatedGASTestActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "SpatialGDKSettings.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

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
 * Then, a second test case is tested with a further iteration.
 * This tests that
 * 1. AReplicatedGASTestActor moves out of the client's interest
 * 2. AReplicatedGASTestActor has all their existing components removed
 * 3. AReplicatedGASTestActor moves into the client's interest
 * 4. The client sees AReplicatedGASTestActor no longer has any components
 * This extra test case is implemented in steps 6.1 and 9.1
 */

const static float StepTimeLimit = 20.0f;

ADynamicSubobjectsTest::ADynamicSubobjectsTest()
	: Super()
{
	Author = "Evi&Arthur&Miron";
	Description = TEXT("Test Dynamic Subobjects Duplication in Client");

	CharacterSpawnLocation = FVector(0.0f, 120.0f, 40.0f);
	CharacterRemoteLocation = FVector(20000.0f, 20000.0f, 40.0f); // Outside of the interest range of the client

	TimeLimit = 200.0f;
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
			if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should be valid"))
				&& PlayerCharacter == GetFlowPlayerController()->AcknowledgedPawn)
			{
				FinishStep();
			}
		},
		StepTimeLimit);

	AddStep(
	TEXT("DynamicSubobjectsTestClientCheckPossesion"), FWorkerDefinition::Client(1), nullptr, nullptr,
	[this](float DeltaTime) {
		RequireCompare_Int(GetNumComponentsOnTestActor(), EComparisonMethod::Greater_Than_Or_Equal_To, 3,
			TEXT("For this test, DynamicSubobjectTestActor should have at least 3 components"));
		FinishStep();
	},
	StepTimeLimit);

	AddStep(
		TEXT("DynamicSubobjectsTestClientCheckPossesion"), FWorkerDefinition::Server(1), nullptr, nullptr,
		[this](float DeltaTime) {
			// server has 1 more component than client (idk why)
			RequireCompare_Int(GetNumComponentsOnTestActor(), EComparisonMethod::Greater_Than_Or_Equal_To, 4,
				TEXT("For this test, DynamicSubobjectTestActor should have at least 4 components"));

			// add new dynamic component to test actor
			TestActor = GetReplicatedTestActor();
			UStaticMeshComponent* createdComp = NewObject<UStaticMeshComponent>(TestActor);
			createdComp->AttachToComponent(TestActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
			createdComp->RegisterComponent();
			createdComp->SetIsReplicated(true);

			RequireCompare_Int(GetNumComponentsOnTestActor(), EComparisonMethod::Greater_Than_Or_Equal_To, 5,
				TEXT("Now DynamicSubobjectTestActor should have at least 5 components"));
			FinishStep();
		},
		StepTimeLimit);

	AddStep(
		TEXT("DynamicSubobjectsTestClientCheckPossesion"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float DeltaTime) {
			RequireCompare_Int(GetNumComponentsOnTestActor(), EComparisonMethod::Greater_Than_Or_Equal_To, 4,
				TEXT("Now DynamicSubobjectTestActor should have at least 4 components"));
			FinishStep();
		},
		StepTimeLimit);

	for (int i = 0; i < DynamicComponentsPerClass + 2; ++i)
	{
		const bool LastStepLoop = i == DynamicComponentsPerClass + 1;

		// Step 3 - Server moves the TestMovementCharacter of Client 1 to a remote location, so that it does not see the
		// AReplicatedGASTestActor.
		AddStep(TEXT("DynamicSubobjectsTestServerMoveClient1"), FWorkerDefinition::Server(1), nullptr, [this]() {
			if (ClientOneSpawnedPawn->SetActorLocation(CharacterRemoteLocation)
				&& RequireEqual_Vector(ClientOneSpawnedPawn->GetActorLocation(), CharacterRemoteLocation,
									   TEXT("Client pawn was not moved to remote location"), 1.0f))
			{
				FinishStep();
			}
		});

		// Step 4 - Client 1 makes sure that the movement was correctly replicated
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckFirstMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				APawn* PlayerCharacter = GetFlowPawn();

				if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should not be nullptr"))
					&& RequireEqual_Vector(PlayerCharacter->GetActorLocation(), CharacterRemoteLocation,
										   TEXT("Character was not moved to remote location"), 1.0f))
				{
					FinishStep();
				}
			},
			StepTimeLimit);

		// when in native, we need to wait for a while here - so the engine can update relevancy
		USpatialNetDriver* CastNetDriver = Cast<USpatialNetDriver>(GetNetDriver());
		if (!CastNetDriver)
		{

			AddStep(TEXT("DynamicSubobjectsTestNativeWaitABit"), FWorkerDefinition::Server(1), nullptr, nullptr,
				[this](float DeltaTime)
				{
					StepTimer += DeltaTime;
					if (StepTimer > 10.0f)
					{
						StepTimer = 0;
						FinishStep();
					}
				}
			);
		}

		// Step 5 - Server increases AReplicatedGASTestActor's TestIntProperty to enable checking if the client is out of interest later.
		AddStep(TEXT("DynamicSubobjectsTestServerIncreasesIntValue"), FWorkerDefinition::Server(1), nullptr, [this, i]() {
			TestActor = GetReplicatedTestActor();
			if (AssertIsValid(TestActor, TEXT("Test actor should be valid")))
			{
				TestActor->TestIntProperty = i + 1;
				FinishStep();
			}
		});



		// Step 6 - Client 1 checks it can no longer see the AReplicatedGASTestActor
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this, i](float DeltaTime) {
				RequireNotEqual_Int(TestActor->TestIntProperty, i + 1, TEXT("Check TestIntProperty didn't get replicated"));
				StepTimer += DeltaTime;
				if (StepTimer >= 0.5f)
				{
					FinishStep();
					StepTimer = 0.0f; // reset for the next time
				}
			},
			StepTimeLimit);

		if (LastStepLoop)
		{
			// step 6.1 - Server removes all components for secondary test case
			AddStep(
				TEXT("DynamicSubobjectsTestServerDestroyActorComponents"), FWorkerDefinition::Server(1), nullptr, nullptr,
				[this](float DeltaTime) {
					TArray<USceneComponent*> AllSceneComps;
					TestActor->GetComponents<USceneComponent>(AllSceneComps);

					RequireCompare_Int(AllSceneComps.Num(), EComparisonMethod::Greater_Than_Or_Equal_To, 2,
									   TEXT("For this test, DynamicSubobjectTestActor should have at least 2 components"));

					// delete all the components on the actor (apart from root)
					const uint32 RootComponentId = TestActor->GetRootComponent()->GetUniqueID();
					for (USceneComponent* SceneComponent : AllSceneComps)
					{
						if (SceneComponent->GetUniqueID() != RootComponentId)
						{
							SceneComponent->DestroyComponent();
						}
					}

					RequireEqual_Int(GetNumComponentsOnTestActor(), 1, TEXT("now gasactor should have 1 component"));

					FinishStep();
				},
				StepTimeLimit);
		}

		// Step7 - Server moves Client 1 close to the cube.
		AddStep(TEXT("DynamicSubobjectsTestServerMoveClient1CloseToCube"), FWorkerDefinition::Server(1), nullptr, [this]() {
			ClientOneSpawnedPawn->SetActorLocation(CharacterSpawnLocation);
			RequireEqual_Vector(ClientOneSpawnedPawn->GetActorLocation(), CharacterSpawnLocation,
				TEXT("Server 1 should see the pawn close to the initial spawn location"), 1.0f);
			FinishStep();
		});

		// Step 8 - Client 1 checks that the movement was replicated correctly.
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckSecondMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this](float DeltaTime) {
				APawn* PlayerCharacter = GetFlowPawn();

				if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should be valid"))
					&& RequireEqual_Vector(PlayerCharacter->GetActorLocation(), CharacterSpawnLocation,
										   TEXT("Client 1 should see themself close to the initial spawn location"), 1.0f))
				{
					FinishStep();
				}
			},
			StepTimeLimit);

		// Step 9 - Client 1 checks it can see the AReplicatedGASTestActor
		AddStep(
			TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased2"), FWorkerDefinition::Client(1), nullptr, nullptr,
			[this, i](float DeltaTime) {
				RequireEqual_Int(TestActor->TestIntProperty, i + 1, TEXT("Client 1 should see the updated TestIntProperty value"));
				FinishStep();
			},
			StepTimeLimit);

		if (LastStepLoop)
		{
			AddStep(
				TEXT("DynamicSubobjectsTestAllWorkers"), FWorkerDefinition::Server(1), nullptr, nullptr,
				[this](float DeltaTime) {
					TestActor = GetReplicatedTestActor();
					if (TestActor)
					{
						// TArray<USceneComponent*> AllSceneComps;
						// TestActor->GetComponents<USceneComponent>(AllSceneComps);
						//
						// RequireCompare_Int(AllSceneComps.Num(), EComparisonMethod::Greater_Than_Or_Equal_To, 1,
						// 				TEXT("For this test, DynamicSubobjectTestActor should have at least 1 component"));
						//
						// // delete all the components on the actor
						// for (USceneComponent* SceneComponent : AllSceneComps)
						// {
						// 	SceneComponent->DestroyComponent();
						// }
						//
						// TArray<USceneComponent*> TwoSceneComps;
						// TestActor->GetComponents<USceneComponent>(TwoSceneComps);
						// RequireEqual_Int(TwoSceneComps.Num(), 0,
						// 				TEXT("now gasactor should have 0 components"));

						FinishStep();
					}
				},
				StepTimeLimit);
		}

		if (LastStepLoop)
		{
			// Step 9.1 - Client 1 checks all components on ReplicatedGASTestActor have been removed
			AddStep(
				TEXT("DynamicSubobjectsTestClientCheckIntValueIncreased2"), FWorkerDefinition::Client(1), nullptr, nullptr,
				[this](float DeltaTime) {

					// RequireEqual_Int(GetNumComponentsOnTestActor(), 1,
									 // TEXT("Every one of DynamicSubobjectTestActor's components (apart from root comp) should have been destroyed."));
					RequireEqual_Int(GetNumComponentsOnTestActor(), 3,
													TEXT("All of DynamicSubobjectTestActor's dynamic components should have been destroyed."));

					FinishStep();
				},
				StepTimeLimit);
		}
	}

	// Step 10 - Server Cleanup.
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
