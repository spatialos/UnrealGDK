// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRepossession.h"

#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialTestPossession.h"
#include "TestPossessionPawn.h"

void ASpatialTestRepossession::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestRepossession, Controllers);
	DOREPLIFETIME(ASpatialTestRepossession, TestPawns);
}

ASpatialTestRepossession::ASpatialTestRepossession()
	: Super()
{
	Author = "Improbable";
	Description = TEXT("Test Actor Repossession");
}

void ASpatialTestRepossession::BeginPlay()
{
	Super::BeginPlay();

	AddStep(TEXT("SpatialTestRepossessionServerSetupStep"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
				ASpatialTestRepossession* Test = Cast<ASpatialTestRepossession>(NetTest);
				ASpatialFunctionalTestFlowController* LocalFlowController = Test->GetLocalFlowController();
				checkf(LocalFlowController, TEXT("Can't be running test without valid FlowControl."));

				Test->TestPawns.Empty();
				Test->Controllers.Empty();

				float YToSpawnAt = -60.0f;
				float YSpawnIncrement = 120.0f;

				for (ASpatialFunctionalTestFlowController* FlowController : Test->GetFlowControllers())
				{
					if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
					{
						continue;
					}
					ATestPossessionPawn* TestPawn = Test->GetWorld()->SpawnActor<ATestPossessionPawn>(
						FVector(0.0f, YToSpawnAt, 50.0f), FRotator::ZeroRotator, FActorSpawnParameters());
					Test->RegisterAutoDestroyActor(TestPawn);

					Test->TestPawns.Add(TestPawn);
					YToSpawnAt += YSpawnIncrement;

					APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
					Test->Controllers.Add(PlayerController);

					// Save original Pawn
					Test->OriginalPawns.Add(TPair<AController*, APawn*>(PlayerController, PlayerController->GetPawn()));

					PlayerController->Possess(TestPawn);
				}

				Test->FinishStep();
			});

	auto ClientCheckPossessionTickLambda = [](ASpatialFunctionalTest* NetTest, float DeltaTime) {
		ASpatialTestRepossession* Test = Cast<ASpatialTestRepossession>(NetTest);
		ASpatialFunctionalTestFlowController* LocalFlowController = Test->GetLocalFlowController();

		Test->AssertTrue(Test->Controllers.Num() == Test->TestPawns.Num(),
						 TEXT("Number of players is equal to the number of pawns spawned"), LocalFlowController);
		APlayerController* PlayerController = Cast<APlayerController>(LocalFlowController->GetOwner());
		bool bFoundCorrectPair = false;
		for (int i = 0; i < Test->TestPawns.Num(); i++)
		{
			if (PlayerController->GetPawn() == Test->TestPawns[i] && PlayerController == Test->Controllers[i])
			{
				bFoundCorrectPair = true;
				break;
			}
		}
		if (bFoundCorrectPair)
		{
			Test->AssertTrue(bFoundCorrectPair, TEXT("Player has possessed correct test pawn"), PlayerController);

			Test->FinishStep();
		}
	};

	AddStep(
		TEXT("SpatialTestRepossessionClientCheckPossessionStep"), FWorkerDefinition::AllClients,
		[](ASpatialFunctionalTest* NetTest) -> bool {
			ASpatialTestRepossession* Test = Cast<ASpatialTestRepossession>(NetTest);
			int NumClients = Test->GetNumRequiredClients();
			return Test->Controllers.Num() == NumClients && Test->TestPawns.Num() == NumClients;
		},
		nullptr, ClientCheckPossessionTickLambda);

	AddStep(TEXT("SpatialTestRepossessionServerSwitchStep"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
				ASpatialTestRepossession* Test = Cast<ASpatialTestRepossession>(NetTest);
				ASpatialFunctionalTestFlowController* LocalFlowController = Test->GetLocalFlowController();
				checkf(LocalFlowController, TEXT("Can't be running test without valid FlowControl."));

				Test->AssertTrue(Test->Controllers.Num() == Test->TestPawns.Num(),
								 TEXT("Number of players is equal to the number of pawns spawned"), LocalFlowController);
				int NumPawns = Test->Controllers.Num();
				APlayerController* FirstController = Test->Controllers[0];
				for (int i = 0; i < NumPawns; i++)
				{
					Test->Controllers[i]->Possess(Test->TestPawns[(i + 1) % NumPawns]);
					if (i > 0)
					{
						Test->Controllers[i - 1] = Test->Controllers[i];
					}
				}
				Test->Controllers[NumPawns - 1] = FirstController;

				Test->FinishStep();
			});

	AddStep(TEXT("SpatialTestRepossessionClientCheckRepossessionStep"), FWorkerDefinition::AllClients, nullptr, nullptr,
			ClientCheckPossessionTickLambda);

	AddStep(TEXT("SpatialTestRepossessionServerPossessOriginalPawn"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
				ASpatialTestRepossession* Test = Cast<ASpatialTestRepossession>(NetTest);
				for (const auto& OriginalPawnPair : Test->OriginalPawns)
				{
					OriginalPawnPair.Key->Possess(OriginalPawnPair.Value);
				}
				Test->FinishStep();
			});
}
