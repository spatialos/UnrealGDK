// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "StaticSubobjectsTest.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/TestMovementCharacter.h"
#include "StaticSubObjectTestActor.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

#include "Components/SceneComponent.h"

#include "EngineClasses/SpatialNetDriver.h"

/**
 * Tests the deletion of replicated static subobjects on an actor by the server while that actor isn't in a client's interest.
 *
 * There are two expected outcomes.
 *
 * Outcome 1.
 *
 * A static subobject can be deleted on the server before the server has ever communicated with the client about that subobject.
 * When the client then gains interest in the actor, the subobject should remain UNDELETED. This is a byproduct of native's implementation which we are matching.
 *
 * Outcome 2.
 *
 * A static subobject is deleted on the server while its actor is not in the client's interest. However the client and the server have previously communicated about the subobject.
 * In this case, when the actor comes back into the clients interest, that subobject should be deleted on the client.
 *
 *
 * The test includes a single server and one client worker.
 *
 * The flow is as follows:
 *  - Setup:
 *	  - One cube actor already placed in the level at Location FVector(-20000.0f, -20000.0f, 40.0f) needs to be a startup actor - bNetLoadOnClient =
 *true.
 *    - The Server spawns a TestMovementCharacter and makes Client 1 possess it.
 *  - Test:
 *    - Each worker tests if the AStaticSubobjectTestActor has indeed been loaded into the map.
 *	  - Server increases the TestIntProperty
 *	  - Client checks it can't see the updated property as its pawn is still at spawn/the origin
 *	  - Server removes a static component from the actor
 *	  - Client moves to the remote location near the actor
 *	  - Client checks it still sees the component on the actor that was removed on the server
 *	  - Client moves back to spawn
 *	  - Server removes another static component
 *	  - Client moves to remote location
 *	  - Client checks that the static component most recently removed by the server has also been removed on the client.
 *
 *  - Cleanup:
 *    - Client repossesses its default pawn.
 *    - The spawned Character is destroyed.
 *
 */

static constexpr float StepTimeLimit = 15.0f;

AStaticSubobjectsTest::AStaticSubobjectsTest()
	: Super()
{
	Author = "Arthur&Miron";
	Description = TEXT("Test Static Subobjects Deletion in Client");
}

void AStaticSubobjectsTest::PrepareTest()
{
	Super::PrepareTest();

	StepTimer = 0.0f;

	// Step 0 - The server spawns a TestMovementCharacter and makes Client 1 possess it.
	AddStep(TEXT("StaticSubobjectsTestSetup"), FWorkerDefinition::Server(1), nullptr, [this]() {
		ASpatialFunctionalTestFlowController* ClientOneFlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		APlayerController* PlayerOneController = Cast<APlayerController>(ClientOneFlowController->GetOwner());

		if (AssertIsValid(PlayerOneController, TEXT("PlayerOneController should be valid")))
		{
			ClientSpawnedPawn = GetWorld()->SpawnActor<ATestMovementCharacter>(PawnSpawnLocation, FRotator::ZeroRotator);
			RegisterAutoDestroyActor(ClientSpawnedPawn);
			ClientDefaultPawn = PlayerOneController->GetPawn();
			PlayerOneController->Possess(ClientSpawnedPawn);

			FinishStep();
		}
	});

	// Step 1 - All workers check for one AStaticSubobjectTestActor in the world and initialise it.
	AddStep(
		TEXT("StaticSubobjectsTestAllWorkersInitialise"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float DeltaTime) {
			TestActor = GetReplicatedTestActor();
			TestActor->InitialiseTestIntProperty();
			FinishStep();
		},
		StepTimeLimit);

	// Step 2 - Client 1 checks if it has correctly possessed the TestMovementCharacter.
	AddStep(
		TEXT("StaticSubobjectsTestClientCheckPawnPossesion"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float DeltaTime) {
			APawn* PlayerPawn = GetFlowPawn();
			if (AssertIsValid(PlayerPawn, TEXT("PlayerCharacter should be valid")))
			{
				RequireTrue(PlayerPawn == GetFlowPlayerController()->AcknowledgedPawn, TEXT("The client should possess the pawn."));
				FinishStep();
			}
		},
		StepTimeLimit);

	// Step 3
	CheckClientNumberComponentsOnTestActorWithoutWait(InitialNumComponents);

	// Step 4
	ServerSetIntProperty(1);

	// Step 5
	CheckClientCanNotSeeIntPropertyWithWait(1);

	// Step 6 - Server removes a static component
	AddStep(TEXT("StaticSubobjectsTestServerDestroyOneSubobject"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AssertEqual_Int(GetNumComponentsOnTestActor(), InitialNumComponents,
						TEXT("AStaticSubobjectTestActor should have the initial number of components"));

		DestroyOneNonRootComponent();

		AssertEqual_Int(GetNumComponentsOnTestActor(), InitialNumComponents-1,
						TEXT("AStaticSubobjectTestActor should have had one component removed"));
		FinishStep();
	});

	// Step 7
	MoveClientPawn(PawnRemoteLocation);

	WaitForRelevancyUpdateIfInNative();

	// Step 8
	CheckClientSeeIntProperty(1);

	// Step 9
	CheckClientNumberComponentsOnTestActorWithoutWait(InitialNumComponents);

	// Step 10
	MoveClientPawn(PawnSpawnLocation);

	WaitForRelevancyUpdateIfInNative();

	// Step 11
	ServerSetIntProperty(2);

	// Step 12
	CheckClientCanNotSeeIntPropertyWithWait(2);

	// Step 12 - Remove another component from the subobject. This time the client should see the removal once the actor is back in its interest
	AddStep(TEXT("StaticSubobjectsTestServerDestroySecondSubobject"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AssertEqual_Int(GetNumComponentsOnTestActor(), InitialNumComponents-1,
						TEXT("AStaticSubobjectTestActor should have one less than the initial number of components on the server."));
		DestroyOneNonRootComponent();
		AssertEqual_Int(GetNumComponentsOnTestActor(), InitialNumComponents-2,
						TEXT("AStaticSubobjectTestActor should have had another component removed"));
		FinishStep();
	});

	// Step 14
	CheckClientNumberComponentsOnTestActorWithWait(InitialNumComponents);

	// Step 15
	MoveClientPawn(PawnRemoteLocation);

	// Step 16
	CheckClientSeeIntProperty(2);

	// Step 17
	CheckClientNumberComponentsOnTestActorWithWait(InitialNumComponents-1);

	// Step 18 - Server Cleanup.
	AddStep(TEXT("StaticSubobjectsTestServerCleanup"), FWorkerDefinition::Server(1), nullptr, [this]() {
		// Possess the original pawn, so that the spawned character can get destroyed correctly
		ASpatialFunctionalTestFlowController* ClientOneFlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		APlayerController* PlayerController = Cast<APlayerController>(ClientOneFlowController->GetOwner());

		if (AssertIsValid(PlayerController, TEXT("PlayerController should be valid")))
		{
			PlayerController->Possess(ClientDefaultPawn);
			FinishStep();
		}
	});
}

void AStaticSubobjectsTest::MoveClientPawn(FVector& ToLocation)
{
	// Server moves Client 1's pawn to the location
	AddStep(TEXT("StaticSubobjectsTestServerMoveClient"), FWorkerDefinition::Server(1), nullptr, [this, &ToLocation]() {
		if (AssertIsValid(ClientSpawnedPawn, TEXT("ClientSpawnedPawn should be valid")))
		{
			ClientSpawnedPawn->SetActorLocation(ToLocation);
			AssertEqual_Vector(ClientSpawnedPawn->GetActorLocation(), ToLocation,
							TEXT("Client pawn should have been moved to location"), 1.0f);
		}
		FinishStep();
	});


	// Client 1 makes sure that the movement was correctly replicated
	AddStep(
		TEXT("StaticSubobjectsTestClientCheckMovement"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this, &ToLocation](float DeltaTime) {
			APawn* PlayerCharacter = GetFlowPawn();

			if (AssertIsValid(PlayerCharacter, TEXT("PlayerCharacter should be valid")))
			{
				RequireEqual_Vector(PlayerCharacter->GetActorLocation(), ToLocation,
									TEXT("Character was not moved to location"), 1.0f);
				FinishStep();
			}
		},
		StepTimeLimit);
}

void AStaticSubobjectsTest::CheckClientCanNotSeeIntPropertyWithWait(int ShouldntSeeVal)
{
	AddStep(
		TEXT("StaticSubobjectsTestClientCheckIntValueDidntIncreaseWithWait"), FWorkerDefinition::Client(1), nullptr,
		[this]() {
			StepTimer = 0.f;
		},
		[this, ShouldntSeeVal](const float DeltaTime) {
			RequireNotEqual_Int(TestActor->TestIntProperty, ShouldntSeeVal, TEXT("The updated TestIntProperty shouldn't have been replicated"));
			StepTimer += DeltaTime;
			if (StepTimer >= 0.5f)
			{
				FinishStep();
			}
		},
		StepTimeLimit);
}

void AStaticSubobjectsTest::CheckClientNumberComponentsOnTestActorWithoutWait(int ExpectedNumComponents)
{
	AddStep(
	TEXT("StaticSubobjectsTestClientSeeRightNumberComponentsWithoutWait"), FWorkerDefinition::Client(1), nullptr,
		[this, ExpectedNumComponents]() {
			AssertEqual_Int(GetNumComponentsOnTestActor(), ExpectedNumComponents,
							TEXT("The client should see the right number of components."));

			FinishStep();
		});
}

void AStaticSubobjectsTest::CheckClientNumberComponentsOnTestActorWithWait(int ExpectedNumComponents)
{
	AddStep(
	TEXT("StaticSubobjectsTestClientSeeRightNumberComponentsWithWait"), FWorkerDefinition::Client(1), nullptr, [this]() {
		StepTimer = 0.f;
	},
		[this, ExpectedNumComponents](const float DeltaTime) {
			RequireEqual_Int(GetNumComponentsOnTestActor(), ExpectedNumComponents,
							TEXT("The client should see the right number of components."));

			StepTimer += DeltaTime;
			if (StepTimer >= 0.5f)
			{
				FinishStep();
			}
		},
		StepTimeLimit);
}

void AStaticSubobjectsTest::ServerSetIntProperty(int IntPropertyNewVal)
{
	AddStep(TEXT("StaticSubobjectsTestServerIncreasesIntValue"), FWorkerDefinition::Server(1), nullptr, [this, IntPropertyNewVal]() {
		TestActor->TestIntProperty = IntPropertyNewVal;
		FinishStep();
	});
}

void AStaticSubobjectsTest::CheckClientSeeIntProperty(int IntPropertyVal)
{
	AddStep(
	TEXT("StaticSubobjectsTestClientCheckIntValueIncreased"), FWorkerDefinition::Client(1), nullptr, nullptr,
	[this, IntPropertyVal](const float DeltaTime) {
		RequireEqual_Int(TestActor->TestIntProperty, IntPropertyVal, TEXT("Client should TestIntProperty see the updated int property"));
		FinishStep();
	},
	StepTimeLimit);
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

void AStaticSubobjectsTest::DestroyOneNonRootComponent() const
{
	TArray<USceneComponent*> AllSceneComps;
	TestActor->GetComponents<USceneComponent>(AllSceneComps);

	const uint32 RootComponentId = TestActor->GetRootComponent()->GetUniqueID();
	for (USceneComponent* SceneComponent : AllSceneComps)
	{
		if (SceneComponent->GetUniqueID() != RootComponentId)
		{
			SceneComponent->DestroyComponent();
			return;
		}
	}
	ensureAlwaysMsgf(false, TEXT("DestroyOneNonRootComponent called when TestActor has no components left to destroy. DestroyOneNonRootComponent should only be called as many times as there are static components on TestActor."));
}

void AStaticSubobjectsTest::WaitForRelevancyUpdateIfInNative()
{
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
}
