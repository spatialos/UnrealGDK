// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "VisibilityTest.h"
#include "ReplicatedVisibilityTestActor.h"
#include "GameFramework/PlayerController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMovement/TestMovementCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Containers/Array.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"

/**
 * This test tests if a bHidden Actor is replicating properly to Server and Clients.
 * 
 * The test includes a single server and two client workers.
 * The flow is as follows:
 *  - Setup:
 *	  - On cube actor already placed in the level
 *    - Two test pawn actors are spawned, one for each client
 *    - The controllers for each client  possess the spawned test pawn actors
 *  - Test:
 *    - Server moves one Client to a remote position where Client can not see the HiddenActor(Cube) .
 *	  - Check if the Clients can see the HiddenActor(Cube).
 *	  - Server sets the HiddenActor(Cube) as bHidden and moves the remote Client back close to the HiddenActor(Cube).
 *	  - Check if the Clients can see the HiddenActor(Cube).
 *  - Cleanup:
 *    - The clients repossess their default pawns
 *    - The test pawns are destroyed
 */

AVisibilityTest::AVisibilityTest()
	: Super()
{
	Author = "Evi";
	Description = TEXT("Test Actor Visibility");
}

void AVisibilityTest::BeginPlay()
{
	Super::BeginPlay();
	PreviousPositionUpdateFrequency = GetDefault<USpatialGDKSettings>()->PositionUpdateFrequency;

	CharacterRemoteLocation = FVector(20000.0f, 20000.0f, 50.0f);
	Character1StartingLocation = FVector(0.0f, 120.0f, 50.0f);
	Character2StartingLocation = FVector(0.0f, 240.0f, 50.0f);

	{	// Step 1 - Set up the world Spawn PlayerCharacter and make sure the cube is in the world
		AddStep(TEXT("ServerSetup"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest) {
			AVisibilityTest* Test = Cast<AVisibilityTest>(NetTest);
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			FVector CubeLocation = FVector::ZeroVector;
			FVector StartingLocation = FVector::ZeroVector;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(GetWorld()); Iter; ++Iter)
			{
				Counter++;
				CubeLocation = Iter->GetActorLocation();
			}
			NetTest->AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in the server world"), NetTest);

			PreviousPositionUpdateFrequency = GetDefault<USpatialGDKSettings>()->PositionUpdateFrequency;
			GetMutableDefault<USpatialGDKSettings>()->PositionUpdateFrequency = 1.0f;
			
			for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
			{
				if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
				{
					continue;
				}

				if (FlowController->WorkerDefinition.Id == 1)
				{
					StartingLocation = Character1StartingLocation;
				}
				else if (FlowController->WorkerDefinition.Id == 2)
				{
					StartingLocation = Character2StartingLocation;
				}

				ATestMovementCharacter* TestCharacter = GetWorld()->SpawnActor<ATestMovementCharacter>(StartingLocation, FRotator::ZeroRotator, FActorSpawnParameters());
				APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
				// Save old one to put it back in the final step
				Test->OriginalPawns.Add(TPair<AController*, APawn*>(PlayerController, PlayerController->GetPawn()));

				RegisterAutoDestroyActor(TestCharacter);
				PlayerController->Possess(TestCharacter);
			}
			NetTest->FinishStep();
		});
	}

	{	// Step 2 - Check if the PlayerContoller exists to all the Clients
		AddStep(TEXT("ClientCheckSpawningFinished"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();

			AController* PlayerController = Cast<AController>(FlowController->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());
			if (PlayerCharacter != nullptr)
			{
				if (PlayerCharacter->HasActiveCameraComponent())
				{
					FinishStep();
				}
			}
		}, 50.0f);
	}

	{	//Step 3 - Server moves Client 1 outside away from the cube 
		AddStep(TEXT("ServerMoveClient1"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest) {

			ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
			APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

			if (PlayerCharacter->GetActorLocation().Equals(Character1StartingLocation, 50.0f))
			{
				PlayerCharacter->SetActorLocation(CharacterRemoteLocation);
				if (PlayerCharacter->GetActorLocation().Equals(CharacterRemoteLocation, 50.0f))
				{
					NetTest->FinishStep();
				}
			}
		});
	}

	{   // Step 4 - Make sure that the Client moved to the correct location
		AddStep(TEXT("ClientCheckFirstMovement"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();

			AController* PlayerController = Cast<AController>(FlowController->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

			if (PlayerCharacter != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("X : %f"), PlayerCharacter->GetActorLocation().X)
				if ((PlayerCharacter->GetActorLocation().X == CharacterRemoteLocation.X) && (PlayerCharacter->GetActorLocation().Y == CharacterRemoteLocation.Y))
				{
					FinishStep();
				}
			}

		}, 50.0f);
	}

	{	// Step 5 - Observe if the test actor has been replicated to the client 1.
		AddStep(TEXT("ClientCheckReplicatedActorsBeforeActorHidden"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime){
			int Counter = 0;
			int ExpectedReplicatedActors = 0;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(NetTest->GetWorld()); Iter; ++Iter)
			{
				Counter++;
			}
			if (Counter == ExpectedReplicatedActors)
			{
				NetTest->AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in client 1 world"), NetTest);
				NetTest->FinishStep();
			}
		}, 100.0f);
	}

	{	// Step 6 - Observe if the test actor has been replicated to the client 2.
		AddStep(TEXT("ClientCheckReplicatedActorsBeforeActorHidden"), FWorkerDefinition::Client(2), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime) {
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(NetTest->GetWorld()); Iter; ++Iter)
			{
				Counter++;
			}
			if (Counter == ExpectedReplicatedActors)
			{
				NetTest->AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in client 2 world"), NetTest);
				NetTest->FinishStep();
			}
		}, 5.0f);
	}

	{	// Step 7 - Server Set Actor Hidden.
		AddStep(TEXT("ServerSetActorHidden"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest) {
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(NetTest->GetWorld()); Iter; ++Iter)
			{
				Counter++;
				Iter->SetHidden(true);
				NetTest->AssertEqual_Bool(Iter->IsHidden(), true, TEXT("TestHiddenActors is Hidden in the server world"), NetTest);
			}
			NetTest->AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in server world"), NetTest);
			NetTest->FinishStep();
		});
	}

	{	// Step 8 - Observe if the test actor has been replicated to the server and move Client 1 back close to the cube.
		AddStep(TEXT("ServerCheckReplicatedActorsAfterSetActorHidden"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest) {
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(NetTest->GetWorld()); Iter; ++Iter)
			{
					Counter++;
			}
			NetTest->AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in the server world"), NetTest);

			ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
			AController* PlayerController = Cast<AController>(FlowController->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

			// Move the character closer to the cube location
			PlayerCharacter->SetActorLocation(Character1StartingLocation);
			if (PlayerCharacter->GetActorLocation().Equals(Character1StartingLocation, 50.0f))
			{
				NetTest->FinishStep();
			}
		});
	}

	{   // Step 9 - Make sure that the Client moved to the correct location
		AddStep(TEXT("ClientCheckSecondMovement"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime){
			ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();
			AController* PlayerController = Cast<AController>(FlowController->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());

			if (PlayerCharacter != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("X : %f, Y : %f"), PlayerCharacter->GetActorLocation().X, PlayerCharacter->GetActorLocation().Y)
				if ((PlayerCharacter->GetActorLocation().X == Character1StartingLocation.X) && (PlayerCharacter->GetActorLocation().Y == Character1StartingLocation.Y))
				{
					FinishStep();
				}
			}
				
		}, 550.0f);
	}

	{	// Step 10 - Observe if the test actor has been replicated to the clients.
		AddStep(TEXT("ClientCheckFinalReplicatedActors"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime) {
			int Counter = 0;
			int ExpectedReplicatedActors = 0;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(NetTest->GetWorld()); Iter; ++Iter)
			{
				Counter++;
				NetTest->AssertEqual_Bool(Iter->IsHidden(), true, TEXT("TestHiddenActors Hidden in the client world"), NetTest);
			}
			if (Counter == ExpectedReplicatedActors)
			{
				NetTest->AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in client world"), NetTest);
				NetTest->FinishStep();
			}
		},5.0f);
	}

	{	// Step 11 - Server Set Actor Hidden False.
		AddStep(TEXT("ServerSetActorNotHidden"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest) {
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(NetTest->GetWorld()); Iter; ++Iter)
			{
				Counter++;
				Iter->SetHidden(false);
				NetTest->AssertEqual_Bool(!Iter->IsHidden(), true, TEXT("TestHiddenActors is Hidden in the server world"), NetTest);
			}
			NetTest->AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in server world"), NetTest);
			NetTest->FinishStep();
		});
	}

	{	// Step 12 - Observe if the test actor has been replicated to the clients.
		AddStep(TEXT("ClientCheckFinalReplicatedNonHiddenActors"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime) {
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(NetTest->GetWorld()); Iter; ++Iter)
			{
				Counter++;
				NetTest->AssertEqual_Bool(!Iter->IsHidden(), true, TEXT("TestHiddenActors Hidden in the client world"), NetTest);
			}
			if (Counter == ExpectedReplicatedActors)
			{
				NetTest->AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in client world"), NetTest);
				NetTest->FinishStep();
			}
		});
	}

	{	// Step 13 - Server Cleanup
		AddStep(TEXT("ServerCleanup"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest) {
			// Possess the original pawn, so that the spawned character can get destroyed correctly
			AVisibilityTest* Test = Cast<AVisibilityTest>(NetTest);
			for (const auto& OriginalPawnPair : Test->OriginalPawns)
			{
				OriginalPawnPair.Key->Possess(OriginalPawnPair.Value);
			}
			Test->FinishStep();
		});
	}
}
