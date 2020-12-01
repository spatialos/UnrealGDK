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

void ASpatialTestRepossession::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("SpatialTestRepossessionServerSetupStep"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		ASpatialFunctionalTestFlowController* LocalFlowController = GetLocalFlowController();
		checkf(LocalFlowController, TEXT("Can't be running test without valid FlowControl."));

		TestPawns.Empty();
		Controllers.Empty();

		float YToSpawnAt = -60.0f;
		float YSpawnIncrement = 120.0f;

		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
			{
				continue;
			}
			ATestPossessionPawn* TestPawn = GetWorld()->SpawnActor<ATestPossessionPawn>(FVector(0.0f, YToSpawnAt, 50.0f),
																						FRotator::ZeroRotator, FActorSpawnParameters());
			RegisterAutoDestroyActor(TestPawn);

			TestPawns.Add(TestPawn);
			YToSpawnAt += YSpawnIncrement;

			APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());
			Controllers.Add(PlayerController);

			// Save original Pawn
			OriginalPawns.Add(TPair<AController*, APawn*>(PlayerController, PlayerController->GetPawn()));

			PlayerController->Possess(TestPawn);
		}

		FinishStep();
	});

	auto ClientCheckPossessionTickLambda = [this](float DeltaTime) {
		ASpatialFunctionalTestFlowController* LocalFlowController = GetLocalFlowController();

		AssertTrue(Controllers.Num() == TestPawns.Num(), TEXT("Number of players is equal to the number of pawns spawned"),
				   LocalFlowController);
		APlayerController* PlayerController = Cast<APlayerController>(LocalFlowController->GetOwner());
		bool bFoundCorrectPair = false;
		for (int i = 0; i < TestPawns.Num(); i++)
		{
			if (PlayerController->GetPawn() == TestPawns[i] && PlayerController == Controllers[i])
			{
				bFoundCorrectPair = true;
				break;
			}
		}
		if (bFoundCorrectPair)
		{
			AssertTrue(bFoundCorrectPair, TEXT("Player has possessed correct test pawn"), PlayerController);

			FinishStep();
		}
	};

	AddStep(
		TEXT("SpatialTestRepossessionClientCheckPossessionStep"), FWorkerDefinition::AllClients,
		[this]() -> bool {
			int NumClients = GetNumRequiredClients();
			return Controllers.Num() == NumClients && TestPawns.Num() == NumClients;
		},
		nullptr, ClientCheckPossessionTickLambda);

	AddStep(TEXT("SpatialTestRepossessionServerSwitchStep"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		ASpatialFunctionalTestFlowController* LocalFlowController = GetLocalFlowController();
		checkf(LocalFlowController, TEXT("Can't be running test without valid FlowControl."));

		AssertTrue(Controllers.Num() == TestPawns.Num(), TEXT("Number of players is equal to the number of pawns spawned"),
				   LocalFlowController);
		int NumPawns = Controllers.Num();
		APlayerController* FirstController = Controllers[0];
		for (int i = 0; i < NumPawns; i++)
		{
			Controllers[i]->Possess(TestPawns[(i + 1) % NumPawns]);
			if (i > 0)
			{
				Controllers[i - 1] = Controllers[i];
			}
		}
		Controllers[NumPawns - 1] = FirstController;

		FinishStep();
	});

	AddStep(TEXT("SpatialTestRepossessionClientCheckRepossessionStep"), FWorkerDefinition::AllClients, nullptr, nullptr,
			ClientCheckPossessionTickLambda);

	AddStep(TEXT("SpatialTestRepossessionServerPossessOriginalPawn"), FWorkerDefinition::Server(1), nullptr, nullptr,
			[this](float DeltaTime) {
				for (const auto& OriginalPawnPair : OriginalPawns)
				{
					OriginalPawnPair.Key->Possess(OriginalPawnPair.Value);
				}
				FinishStep();
			});
}
