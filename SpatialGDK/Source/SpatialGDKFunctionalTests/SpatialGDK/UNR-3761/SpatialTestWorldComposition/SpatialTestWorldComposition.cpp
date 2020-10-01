// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestWorldComposition.h"

#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/InitiallyDormantTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"

#include "Kismet/GameplayStatics.h"

/**
 * This test automates the World Composition Gym which tested level loading and unloading. This test requires to be ran inside a custom map,
 * trying to run it in another map will make the test fail. NOTE: Currently, the test will fail if ran with Native networking, due to the
 * issue described in UNR-4066.
 *
 * The test includes 1 server and 1 client, with all test logic ran only by the Client.
 * The flow is as follows:
 * - Setup:
 *  - The Client sets a reference to its Pawn and resets the TestLocation index.
 * - Test:
 *  - The test contains 2 runs of the same flow:
 *  - The Client moves its Pawn to each TestLocation.
 *  - The Client checks its Pawn has arrived at the correct location and checks if the corresponding level was loaded correctly, and all
 * previously loaded levels were unloaded.
 * - Clean-up:
 *  - No clean-up is required.
 */
ASpatialTestWorldComposition::ASpatialTestWorldComposition()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test World Composition");

	TArray<FExpectedActor> ExpectedConditionsAtStep0;
	ExpectedConditionsAtStep0.Add(FExpectedActor{ FVector(330.0f, -1200.0f, 80.0f), AInitiallyDormantTestActor::StaticClass() });

	TArray<FExpectedActor> ExpectedConditionsAtStep1;
	ExpectedConditionsAtStep1.Add(FExpectedActor{ FVector(330.0f, -400.0f, 80.0f), AReplicatedTestActorBase::StaticClass() });

	TArray<FExpectedActor> ExpectedConditionsAtStep2;
	ExpectedConditionsAtStep2.Add(FExpectedActor{ FVector(330.0f, 400.0f, 80.0f), AReplicatedTestActorBase::StaticClass() });

	TArray<FExpectedActor> ExpectedConditionsAtStep3;
	ExpectedConditionsAtStep3.Add(FExpectedActor{ FVector(330.0f, 1200.0f, 80.0f), AInitiallyDormantTestActor::StaticClass() });

	TArray<FExpectedActor> ExpectedConditionsAtStep4;
	ExpectedConditionsAtStep4.Add(ExpectedConditionsAtStep1[0]);
	ExpectedConditionsAtStep4.Add(ExpectedConditionsAtStep2[0]);

	TestStepsData.Add(TPair<FVector, TArray<FExpectedActor>>(FVector(-150.0f, -1200.0f, 60.0f), ExpectedConditionsAtStep0));
	TestStepsData.Add(TPair<FVector, TArray<FExpectedActor>>(FVector(-150.0f, -400.0f, 60.0f), ExpectedConditionsAtStep1));
	TestStepsData.Add(TPair<FVector, TArray<FExpectedActor>>(FVector(-150.0f, 400.0f, 60.0f), ExpectedConditionsAtStep2));
	TestStepsData.Add(TPair<FVector, TArray<FExpectedActor>>(FVector(-150.0f, 1200.0f, 60.0f), ExpectedConditionsAtStep3));
	TestStepsData.Add(TPair<FVector, TArray<FExpectedActor>>(FVector(0.0f, 0.0f, 60.0f), ExpectedConditionsAtStep4));

	SetNumRequiredClients(1);
}

void ASpatialTestWorldComposition::PrepareTest()
{
	Super::PrepareTest();

	// Step definition for Client 1 to move its Pawn and check if the levels loaded correctly.
	FSpatialFunctionalTestStepDefinition ClientCheckLocationStepDefinition(/*bIsNativeDefinition*/ true);
	ClientCheckLocationStepDefinition.StepName = TEXT("SpatialTestWorldCompositionClientCheckLocation");
	ClientCheckLocationStepDefinition.TimeLimit = 10.0f;
	ClientCheckLocationStepDefinition.NativeStartEvent.BindLambda([this]() {
		ClientOnePawn->SetActorLocation(TestStepsData[TestLocationIndex].Key);
	});
	ClientCheckLocationStepDefinition.NativeTickEvent.BindLambda([this](float DeltaTime) {
		if (IsCorrectAtLocation(TestLocationIndex))
		{
			TestLocationIndex++;
			FinishStep();
		}
	});

	// Run through each of the test locations twice to ensure that levels can be loaded and unloaded successfully multiple times.
	for (int i = 0; i < 2; ++i)
	{
		// Setup step that sets a reference to the Pawn and resets the TestLocationIndex.
		AddStep(TEXT("SpatialTestWorldCompositionClientSetupStep"), FWorkerDefinition::Client(1), nullptr, [this]() {
			ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();
			APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
			ClientOnePawn = PlayerController->GetPawn();
			TestLocationIndex = 0;

			FinishStep();
		});

		// Move the Pawn to the TestLocation with index 0 and test that the InitiallyDormantActorLevel was loaded correctly and the
		// ReplicatedActorLevel and ReplicatedAndNetLoadOnClientLevel were unloaded. Note that since initially, the pawn spawns close to the
		// origin, it will load the ReplicatedActorLevel and ReplicatedAndNetLoadOnClientLevel, failing to unload those will first be seen
		// in this step.
		AddStepFromDefinition(ClientCheckLocationStepDefinition, FWorkerDefinition::Client(1));

		// Move the Pawn to the TestLocation with index 1 and test that the ReplicatedActorLevel was loaded correctly and the
		// IntiallyDormantActorLevel was unloaded.
		AddStepFromDefinition(ClientCheckLocationStepDefinition, FWorkerDefinition::Client(1));

		// Move the Pawn to the TestLocation with index 2 and test that the ReplicatedAndNetLoadOnClientLevel was loaded correctly and the
		// ReplicatedActorLevel was unloaded.
		AddStepFromDefinition(ClientCheckLocationStepDefinition, FWorkerDefinition::Client(1));

		// Move the Pawn to the TestLocation with index 3 and test that the InitiallyDormantAndNetLoadLevel was loaded correctly and the
		// ReplicatedAndNetLoadOnClientLevel was unloaded.
		AddStepFromDefinition(ClientCheckLocationStepDefinition, FWorkerDefinition::Client(1));

		// Move the Pawn to the TestLocation with index 4 and test that both ReplicatedActorLevel and ReplicatedAndNetLoadOnClientLevel were
		// loaded correctly, and that the InitiallyDormantAndNetLoadLevel was unloaded.
		AddStepFromDefinition(ClientCheckLocationStepDefinition, FWorkerDefinition::Client(1));
	}
}

bool ASpatialTestWorldComposition::IsCorrectAtLocation(int TestLocation)
{
	// Check that the movement was correctly applied before checking if levels loaded correctly.
	if (!ClientOnePawn->GetActorLocation().Equals(TestStepsData[TestLocation].Key, 1.0f))
	{
		return false;
	}

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), FoundReplicatedBaseActors);
	TArray<FExpectedActor> ExpectedLocationConditions = TestStepsData[TestLocation].Value;

	if (ExpectedLocationConditions.Num() != FoundReplicatedBaseActors.Num())
	{
		return false;
	}

	int CorrectActors = 0;

	for (AActor* FoundActor : FoundReplicatedBaseActors)
	{
		for (auto Condition : ExpectedLocationConditions)
		{
			if (FoundActor->GetActorLocation().Equals(Condition.ExpectedActorLocation, 1.0f)
				&& FoundActor->IsA(Condition.ExpectedActorClass))
			{
				CorrectActors++;
				break;
			}
		}
	}

	if (CorrectActors != ExpectedLocationConditions.Num())
	{
		return false;
	}

	return true;
}
