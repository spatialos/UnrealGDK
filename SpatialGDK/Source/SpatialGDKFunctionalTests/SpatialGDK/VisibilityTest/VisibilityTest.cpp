// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "VisibilityTest.h"
#include "Containers/Array.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "ReplicatedVisibilityTestActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMovement/TestMovementCharacter.h"

/**
 * This test tests if a bHidden Actor is replicating properly to Server and Clients.
 * 
 * The test includes a single server and two client workers.
 * The flow is as follows:
 *  - Setup:
 *	  - On cube actor already placed in the level Location FVector(0.0f, 0.0f, 50.0f)
 *    - Two test character pawns are spawned, one for each client
 *    - The controllers for each client  possess the spawned test character pawns
 *  - Test:
 *    - Server moves one Client to a remote position where Client can not see the HiddenActor(Cube) .
 *	  - Check if the Clients can see the HiddenActor(Cube).
 *	  - Server sets the HiddenActor(Cube) as bHidden.
 *	  - Server moves the remote Client back close to the HiddenActor(Cube).
 *	  - Check if the Clients can see the HiddenActor(Cube).
 *	  - Server sets the HiddenActor(Cube) as Visible (bHidden = false).
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

void AVisibilityTest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVisibilityTest, Controllers);
	DOREPLIFETIME(AVisibilityTest, TestPawns);
}

void AVisibilityTest::BeginPlay()
{
	Super::BeginPlay();

	CharacterRemoteLocation = FVector(20000.0f, 20000.0f, 50.0f);
	Character1StartingLocation = FVector(0.0f, 120.0f, 50.0f);

	{	// Step 1 - Set up the world Spawn PlayerCharacter and make sure the cube is in the world.
		AddStep(TEXT("ServerSetup"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest)
		{
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			FVector StartingLocation = FVector::ZeroVector;
			float YToSpawnAt = 120.0f;
			float YSpawnIncrement = 120.0f;

			TestPawns.Empty();
			Controllers.Empty();

			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(GetWorld()); Iter; ++Iter)
			{
				Counter++;
			}
			AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in the server world"), NetTest);

			for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
			{
				checkf(FlowController, TEXT("Can't be running test without valid FlowControl."));

				if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
				{
					continue;
				}
				
				StartingLocation = FVector(0.0f, YToSpawnAt, 50.0f);
				ATestMovementCharacter* TestCharacter = GetWorld()->SpawnActor<ATestMovementCharacter>(StartingLocation, FRotator::ZeroRotator, FActorSpawnParameters());
				YToSpawnAt += YSpawnIncrement;

				APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
				Controllers.Add(PlayerController);

				TestPawns.Add(TestCharacter);

				RegisterAutoDestroyActor(TestCharacter);

				// Save old one to put it back in the final step
				OriginalPawns.Add(TPair<AController*, APawn*>(PlayerController, PlayerController->GetPawn()));

				PlayerController->Possess(TestCharacter);

			}
			FinishStep();
		});
	}

	{	// Step 2 - Check if the Character pawn is properly set in all clients.
		AddStep(TEXT("ClientCheckPossesionFinished"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();
			checkf(FlowController, TEXT("Can't be running test without valid FlowControl."));

			APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
			if(IsValid(PlayerController))
			{
					if (TestPawns[FlowController->WorkerDefinition.Id - 1] == PlayerController->AcknowledgedPawn)
					{
						AssertTrue(true, TEXT("Player pawn is set properly"), PlayerController);
						FinishStep();
					}
			}
		}, 5.0f);
	}

	{	// Step 3 - Server moves Client 1 to a remote location where it can not see the cube. 
		AddStep(TEXT("ServerMoveClient1"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest)
		{
			ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
			checkf(FlowController, TEXT("Can't be running test without valid FlowControl."));
			APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());
			if (PlayerCharacter->GetActorLocation().Equals(Character1StartingLocation, 50.0f))
			{
				if (PlayerCharacter->SetActorLocation(CharacterRemoteLocation))
				{
					if (PlayerCharacter->GetActorLocation().Equals(CharacterRemoteLocation, 50.0f))
					{
						FinishStep();
					}
				}
			}
		});
	}

	{   // Step 4 - Make sure that the Client moved to the correct location.
		AddStep(TEXT("ClientCheckFirstMovement"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();
			checkf(FlowController, TEXT("Can't be running test without valid FlowControl."));

			APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());
			if (IsValid(PlayerCharacter))
			{
				if (PlayerCharacter->GetActorLocation().Equals(CharacterRemoteLocation, 50.0f))
				{
					FinishStep();
				}
			}

		}, 5.0f);
	}

	{	// Step 5 - Observe if the test actor has been replicated to the client 1.
		AddStep(TEXT("ClientCheckReplicatedActorsBeforeActorHidden"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			int Counter = 0;
			int ExpectedReplicatedActors = 0;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(NetTest->GetWorld()); Iter; ++Iter)
			{
				Counter++;
			}
			if (Counter == ExpectedReplicatedActors)
			{
				AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in client 1 world"), NetTest);
				FinishStep();
			}
		}, 5.0f);
	}

	{	// Step 6 - Observe if the test actor has been replicated to the client 2.
		AddStep(TEXT("ClientCheckReplicatedActorsBeforeActorHidden"), FWorkerDefinition::Client(2), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(GetWorld()); Iter; ++Iter)
			{
				Counter++;
			}
			if (Counter == ExpectedReplicatedActors)
			{
				AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in client 2 world"), NetTest);
				FinishStep();
			}
		}, 5.0f);
	}

	{	// Step 7 - Server Set Actor Hidden.
		AddStep(TEXT("ServerSetActorHidden"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest) {
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(GetWorld()); Iter; ++Iter)
			{
				Counter++;
				Iter->SetHidden(true);
				AssertEqual_Bool(Iter->IsHidden(), true, TEXT("TestHiddenActors is Hidden in the server world"), NetTest);
			}
			AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in server world"), NetTest);
			FinishStep();
		});
	}

	{	// Step 8 - Observe if the test actor has been replicated to the server.
		AddStep(TEXT("ServerCheckReplicatedActorsAfterSetActorHidden"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest)
		{
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(GetWorld()); Iter; ++Iter)
			{
					Counter++;
			}
			AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in the server world"), NetTest);
			FinishStep();
		});
	}

	{	// Step 9 - Observe if the test actor has been replicated to the Clients.
		AddStep(TEXT("ClientCheckReplicatedActorsAfterSetActorHidden"), FWorkerDefinition::AllClients, nullptr, [this](ASpatialFunctionalTest* NetTest)
		{
			int Counter = 0;
			int ExpectedReplicatedActors = 0;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(GetWorld()); Iter; ++Iter)
			{
				Counter++;
			}
			AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in the client world"), NetTest);
			FinishStep();
		});
	}

	{	// Step 10 - Server moves Client 1 close to the cube.
		AddStep(TEXT("ServerMoveClient1CloseToCube"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest)
		{

			ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
			checkf(FlowController, TEXT("Can't be running test without valid FlowControl."));
			APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());
			if (PlayerCharacter->GetActorLocation().Equals(CharacterRemoteLocation, 50.0f))
			{
				if (PlayerCharacter->SetActorLocation(Character1StartingLocation))
				{
					if (PlayerCharacter->GetActorLocation().Equals(Character1StartingLocation, 50.0f))
					{
						FinishStep();
					}
				}
			}
		});
	}

	{   // Step 11 - Make sure that the Client moved to the correct location.
		AddStep(TEXT("ClientCheckSecondMovement"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();
			checkf(FlowController, TEXT("Can't be running test without valid FlowControl."));

			APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
			ATestMovementCharacter* PlayerCharacter = Cast<ATestMovementCharacter>(PlayerController->GetPawn());
			if (IsValid(PlayerCharacter))
			{
				if (PlayerCharacter->GetActorLocation().Equals(Character1StartingLocation, 50.0f))
				{
					FinishStep();
				}
			}
				
		}, 5.0f);
	}

	{	// Step 12 - Observe if the test actor has been replicated to the clients.
		AddStep(TEXT("ClientCheckFinalReplicatedActors"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			int Counter = 0;
			int ExpectedReplicatedActors = 0;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(GetWorld()); Iter; ++Iter)
			{
				Counter++;
				AssertEqual_Bool(Iter->IsHidden(), true, TEXT("TestHiddenActors Hidden in the client world"), NetTest);
			}
			if (Counter == ExpectedReplicatedActors)
			{
				AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in client world"), NetTest);
				FinishStep();
			}
		},5.0f);
	}

	{	// Step 13 - Server Set Actor Hidden False.
		AddStep(TEXT("ServerSetActorNotHidden"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest)
		{
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(GetWorld()); Iter; ++Iter)
			{
				Counter++;
				Iter->SetHidden(false);
				AssertEqual_Bool(!Iter->IsHidden(), true, TEXT("TestHiddenActors is Hidden in the server world"), NetTest);
			}
			AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in server world"), NetTest);
			FinishStep();
		});
	}

	{	// Step 14 - Observe if the test actor has been replicated to the clients.
		AddStep(TEXT("ClientCheckFinalReplicatedNonHiddenActors"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			int Counter = 0;
			int ExpectedReplicatedActors = 1;
			for (TActorIterator<AReplicatedVisibilityTestActor> Iter(GetWorld()); Iter; ++Iter)
			{
				Counter++;
				AssertEqual_Bool(!Iter->IsHidden(), true, TEXT("TestHiddenActors Hidden in the client world"), NetTest);
			}
			if (Counter == ExpectedReplicatedActors)
			{
				AssertEqual_Int(Counter, ExpectedReplicatedActors, TEXT("Number of TestHiddenActors in client world"), NetTest);
				FinishStep();
			}
		});
	}

	{	// Step 15 - Server Cleanup.
		AddStep(TEXT("ServerCleanup"), FWorkerDefinition::Server(1), nullptr, [this](ASpatialFunctionalTest* NetTest)
		{
			// Possess the original pawn, so that the spawned character can get destroyed correctly
			for (const auto& OriginalPawnPair : OriginalPawns)
			{
				OriginalPawnPair.Key->Possess(OriginalPawnPair.Value);
			}
			FinishStep();
		});
	}
}
