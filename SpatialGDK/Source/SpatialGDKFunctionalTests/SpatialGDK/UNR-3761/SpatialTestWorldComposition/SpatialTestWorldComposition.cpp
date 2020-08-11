// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestWorldComposition.h"

#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/InitiallyDormantTestActor.h"

#include "Kismet/GameplayStatics.h"

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
}

void ASpatialTestWorldComposition::BeginPlay()
{
	Super::BeginPlay();

	// Step definition for Client 1 to move its Pawn
	FSpatialFunctionalTestStepDefinition ClientMoveCharacterStepDefinition;
	ClientMoveCharacterStepDefinition.bIsNativeDefinition = true;
	ClientMoveCharacterStepDefinition.TimeLimit = 5.0f;
	ClientMoveCharacterStepDefinition.NativeStartEvent.BindLambda([this](ASpatialFunctionalTest* NetTest)
		{
			ClientOnePawn->SetActorLocation(TestLocations[TestLocationIndex]);
			TestLocationIndex++;

			FinishStep();
		});

	// Run through each of the test locations twice to ensure that levels can be loaded and unloaded successfully multiple times.
	for (int i = 0; i < 2; ++i)
	{
		AddStep(TEXT("SpatialTestWorldCompositionClientSetupStep"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				ASpatialFunctionalTestFlowController* FlowController = GetLocalFlowController();
				APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
				ClientOnePawn = PlayerController->GetPawn();
				TestLocationIndex = 0;

				FinishStep();
			}, 5.0f);

		AddStepFromDefinition(ClientMoveCharacterStepDefinition, FWorkerDefinition::Client(1));

		// Test that the InitiallyDormantActorLevel was loaded correctly and the ReplicatedActorLevel and ReplicatedAndNetLoadOnClientLevel were unloaded.
		AddStep(TEXT("SpatialTestWorldCompositionClientCheck"), FWorkerDefinition::Client(1), nullptr, nullptr, [this, i](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				// Check that the movement was correctly applied before checking the level loaded correctly
				if (ClientOnePawn->GetActorLocation().Equals(TestLocations[0], 1.0f))
				{
					UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), FoundReplicatedBaseActors);

					if (FoundReplicatedBaseActors.Num() == 1 && FoundReplicatedBaseActors[0]->IsA(AInitiallyDormantTestActor::StaticClass()))
					{
						if (FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[0], 10.0f))
						{
							FinishStep();
						}
					}
				}
			}, 10.0f);

		AddStepFromDefinition(ClientMoveCharacterStepDefinition, FWorkerDefinition::Client(1));

		// Test that the ReplicatedActorLevel was loaded correctly and the IntiallyDormantActorLevel was unloaded.
		AddStep(TEXT("SpatialTestWorldCompositionClientCheck"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				if (ClientOnePawn->GetActorLocation().Equals(TestLocations[1], 1.0f))
				{
					UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), FoundReplicatedBaseActors);

					if (FoundReplicatedBaseActors.Num() == 1 && FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[1], 10.0f))
					{
						FinishStep();
					}
				}
			}, 10.0f);

		AddStepFromDefinition(ClientMoveCharacterStepDefinition, FWorkerDefinition::Client(1));

		// Test that the ReplicatedAndNetLoadOnClientLevel was loaded correctly and the ReplicatedActorLevel was unloaded.
		AddStep(TEXT("SpatialTestWorldCompositionClientCheck"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				if (ClientOnePawn->GetActorLocation().Equals(TestLocations[2], 1.0f))
				{
					UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), FoundReplicatedBaseActors);

					if (FoundReplicatedBaseActors.Num() == 1 && FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[2], 10.0f))
					{
						FinishStep();
					}
				}
			}, 5.0f);

		AddStepFromDefinition(ClientMoveCharacterStepDefinition, FWorkerDefinition::Client(1));

		// Test that the InitiallyDormantAndNetLoadLevel was loaded correctly and the ReplicatedAndNetLoadOnClientLevel was unloaded.
		AddStep(TEXT("SpatialTestWorldCompositionClientCheck"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				if (ClientOnePawn->GetActorLocation().Equals(TestLocations[3], 1))
				{
					UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), FoundReplicatedBaseActors);

					if (FoundReplicatedBaseActors.Num() == 1 && FoundReplicatedBaseActors[0]->IsA(AInitiallyDormantTestActor::StaticClass()))
					{
						if (FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[3], 10.0f))
						{
							FinishStep();
						}
					}
				}
			}, 5.0f);

		AddStepFromDefinition(ClientMoveCharacterStepDefinition, FWorkerDefinition::Client(1));

		// Test that both ReplicatedActorLevel and ReplicatedAndNetLoadOnClientLevel were loaded correctly, and that the InitiallyDormantAndNetLoadLevel was unloaded.
		AddStep(TEXT("SpatialTestWorldCompositionClientCheck"), FWorkerDefinition::Client(1), nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
			{
				if (ClientOnePawn->GetActorLocation().Equals(TestLocations[4], 1))
				{
					UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), FoundReplicatedBaseActors);

					if (FoundReplicatedBaseActors.Num() == 2)
					{
						if(FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[1], 10.0f) && FoundReplicatedBaseActors[1]->GetActorLocation().Equals(ActorsLocations[2], 10.0f)
						|| FoundReplicatedBaseActors[1]->GetActorLocation().Equals(ActorsLocations[1], 10.0f) && FoundReplicatedBaseActors[0]->GetActorLocation().Equals(ActorsLocations[2], 10.0f))
						{ 
							FinishStep();
						}
					}
				}
			}, 5.0f);
	}
}
