// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestWorldComposition.h"

#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/InitiallyDormantTestActor.h"

#include "Kismet/GameplayStatics.h"

/**
 * This test automates the World Composition Gym which tested level loading and unloading. This test requires to be ran inside a custom map, trying to run it in another map will make the test fail.
 * NOTE: Currently, the test will fail if ran with Native networking, due to the issue described in UNR-4066.
 *
 * The test includes 1 server and 1 client, with all test logic ran only by the Client.
 * The flow is as follows:
 * - Setup:
 *  - The Client sets a reference to its Pawn and resets the TestLocation index.
 * - Test:
 *  - The test contains 2 runs of the same flow:
 *  - The Client moves its Pawn to each TestLocation.
 *  - The Client checks its Pawn has arrived at the correct location and checks if the corresponding level was loaded correctly, and all previously loaded levels were unloaded.
 * - Clean-up:
 *  - No clean-up is required.
 */
ASpatialTestWorldComposition::ASpatialTestWorldComposition()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test World Composition");

	TestLocations.Add(FVector(-150.0f, -1200.0f, 60.0f));
	TestLocations.Add(FVector(-150.0f, -400.0f, 60.0f));
	TestLocations.Add(FVector(-150.0f, 400.0f, 60.0f));
	TestLocations.Add(FVector(-150.0f, 1200.0f, 60.0f));
	TestLocations.Add(FVector(0.0f, 0.0f, 60.0f));

	ActorsLocations.Add(FVector(330.0f, -1200.0f, 80.0f));
	ActorsLocations.Add(FVector(330.0f, -400.0f, 80.0f));
	ActorsLocations.Add(FVector(330.0f, 400.0f, 80.0f));
	ActorsLocations.Add(FVector(330.0f, 1200.0f, 80.0f));

	SetNumRequiredClients(1);
}

void ASpatialTestWorldComposition::BeginPlay()
{
	Super::BeginPlay();

	// Step definition for Client 1 to move its Pawn
	FSpatialFunctionalTestStepDefinition ClientMovePawnStepDefinition;
	ClientMovePawnStepDefinition.bIsNativeDefinition = true;
	ClientMovePawnStepDefinition.TimeLimit = 5.0f;
	ClientMovePawnStepDefinition.NativeStartEvent.BindLambda([this](ASpatialFunctionalTest* NetTest)
		{
			ClientOnePawn->SetActorLocation(TestLocations[TestLocationIndex]);
			TestLocationIndex++;

			FinishStep();
		});

	// Run through each of the test locations twice to ensure that levels can be loaded and unloaded successfully multiple times.
	for (int i = 0; i < 2; ++i)
	{
		// Setup step that sets a reference to the Pawn and resets the TestLocationIndex.
		AddStep(TEXT("SpatialTestWorldCompositionClientSetupStep"), FWorkerDefinition::Client(1), nullptr, [this](ASpatialFunctionalTest* NetTest)
			{
				ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();
				APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
				ClientOnePawn = PlayerController->GetPawn();
				TestLocationIndex = 0;

				FinishStep();
			});

		// Move the Pawn to the TestLocation with index 0.
		AddStepFromDefinition(ClientMovePawnStepDefinition, FWorkerDefinition::Client(1));

		// Note that since initially, the pawn spawns close to the origin, it will load the ReplicatedActorLevel and ReplicatedAndNetLoadOnClientLevel, failing to unload those will first be seen in this step. 
		// Test that the InitiallyDormantActorLevel was loaded correctly and the ReplicatedActorLevel and ReplicatedAndNetLoadOnClientLevel were unloaded.
		AddStep(TEXT("SpatialTestWorldCompositionClientCheckTestLocation0"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				if (bIsCorrectAtLocation(0))
				{
					FinishStep();
				}
			}, 10.0f);

		// Move the Pawn to the TestLocation with index 1.
		AddStepFromDefinition(ClientMovePawnStepDefinition, FWorkerDefinition::Client(1));

		// Test that the ReplicatedActorLevel was loaded correctly and the IntiallyDormantActorLevel was unloaded.
		AddStep(TEXT("SpatialTestWorldCompositionClientCheckTestLocation1"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				if (bIsCorrectAtLocation(1))
				{
					FinishStep();
				}
			}, 10.0f);

		// Move the Pawn to the TestLocation with index 2.
		AddStepFromDefinition(ClientMovePawnStepDefinition, FWorkerDefinition::Client(1));

		// Test that the ReplicatedAndNetLoadOnClientLevel was loaded correctly and the ReplicatedActorLevel was unloaded.
		AddStep(TEXT("SpatialTestWorldCompositionClientCheckTestLocation2"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				if (bIsCorrectAtLocation(2))
				{
					FinishStep();
				}
			}, 10.0f);

		// Move the Pawn to the TestLocation with index 3.
		AddStepFromDefinition(ClientMovePawnStepDefinition, FWorkerDefinition::Client(1));

		// Test that the InitiallyDormantAndNetLoadLevel was loaded correctly and the ReplicatedAndNetLoadOnClientLevel was unloaded.
		AddStep(TEXT("SpatialTestWorldCompositionClientCheckTestLocation3"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				if (bIsCorrectAtLocation(3))
				{
					FinishStep();
				}
			}, 10.0f);

		// Move the Pawn to the TestLocation with index 4.
		AddStepFromDefinition(ClientMovePawnStepDefinition, FWorkerDefinition::Client(1));

		// Test that both ReplicatedActorLevel and ReplicatedAndNetLoadOnClientLevel were loaded correctly, and that the InitiallyDormantAndNetLoadLevel was unloaded.
		AddStep(TEXT("SpatialTestWorldCompositionClientCheckTestLocation4"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				if (bIsCorrectAtLocation(4))
				{
					FinishStep();
				}
			}, 10.0f);
	}
}

bool ASpatialTestWorldComposition::bIsCorrectAtLocation(int TestLocation)
{
	// Check that the movement was correctly applied before checking if levels loaded correctly.
	if (ClientOnePawn->GetActorLocation().Equals(TestLocations[TestLocation], 1.0f))
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), FoundReplicatedBaseActors);
		switch (TestLocation)
		{
			case 0:
				if (FoundReplicatedBaseActors.Num() == 1 && FoundReplicatedBaseActors[0]->IsA(AInitiallyDormantTestActor::StaticClass()))
				{
					if (FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[0], 10.0f))
					{
						return true;
					}
				}
				break;
			case 1:
				if (FoundReplicatedBaseActors.Num() == 1 && FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[1], 10.0f))
				{
					return true;
				}
				break;
			case 2:
				if (FoundReplicatedBaseActors.Num() == 1 && FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[2], 10.0f))
				{
					return true;
				}
				break;
			case 3:
				if (FoundReplicatedBaseActors.Num() == 1 && FoundReplicatedBaseActors[0]->IsA(AInitiallyDormantTestActor::StaticClass()))
				{
					if (FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[3], 10.0f))
					{
						return true;
					}
				}
				break;
			case 4:
				if (FoundReplicatedBaseActors.Num() == 2)
				{
					if (FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[1], 10.0f) && FoundReplicatedBaseActors[1]->GetActorLocation().Equals(ActorsLocations[2], 10.0f)
					||  FoundReplicatedBaseActors[1]->GetActorLocation().Equals(ActorsLocations[1], 10.0f) && FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[2], 10.0f))
					{
						return true;
					}
				}
				break;
		}
	}

	return false;
}
