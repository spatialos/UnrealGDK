// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestMultipleOwnership.h"
#include "MultipleOwnershipPawn.h"
#include "SpatialFunctionalTestFlowController.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

/**
 * This test automates the MultipleOwnesrhip gym which demonstrates that RPCs can be sent from multiple actors that have their owner set to
 * a player controller. The test contains 1 Server and 2 Clients.
 *
 * The flow is as follows:
 * - Setup:
 *  - The server spawns 2 MultipleOwnershipPawns and registers them for auto-destroy.
 *  - Every worker sets a reference to the spawned Pawns.
 * - Test:
 *  - Client 1 sends an RPC from both MultipleOwnershipPawns, which should be ignored.
 *  - All workers check that the RPC was correctly ignored.
 *  - The server sets Client 1's PlayerController to possess the first MultipleOnwershipPawn.
 *  - Client 1 sends an RPC from both MultipleOwnershipPawns.
 *  - All workers check that the possessed MultipleOwnershipPawn's RPC was correctly applied, whilst the unpossessed's one was ignored.
 *  - The server sets Client 1's PlayerController to possess the second MultipleOnwershipPawn.
 *  - Client 1 sends an RPC from both MultipleOwnershipPawns.
 *  - All workers check that both RPCs were correctly applied.
 *  - The server sets Client 1's PlayerController to unpossess the MultipleOwnershipPawn;
 *  - Client 1 sends an RPC from both MultipleOwnershipPawns.
 *  - All workers check that both RPCs were correctly applied.
 * - Cleanup
 *  - The MultipleOwnershipPawns are destroyed.
 */

ASpatialTestMultipleOwnership::ASpatialTestMultipleOwnership()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test Net Reference");
}

void ASpatialTestMultipleOwnership::PrepareTest()
{
	Super::PrepareTest();
	// We expect three RPCs to miss, two on the first send (before any ownership of pawns happens), then one on the second send (after the
	// first pawn becomes owned), then zero for the next sends.
	if (HasAuthority())
	{
		AddExpectedLogError(TEXT("No owning connection for actor MultipleOwnershipPawn"), 3, false);
	}

	// The server spawns the 2 MultipleOwnershipPawns and registers them for auto-destroy
	AddStep(TEXT("SpatialTestMultipleOwnershipServerSpawnPawns"), FWorkerDefinition::Server(1), nullptr, [this]() {
		AMultipleOwnershipPawn* MultipleOwnershipPawn1 =
			GetWorld()->SpawnActor<AMultipleOwnershipPawn>(FVector(200.0f, 300.0f, 60.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		AMultipleOwnershipPawn* MultipleOwnershipPawn2 =
			GetWorld()->SpawnActor<AMultipleOwnershipPawn>(FVector(200.0f, -300.0f, 60.0f), FRotator::ZeroRotator, FActorSpawnParameters());

		RegisterAutoDestroyActor(MultipleOwnershipPawn1);
		RegisterAutoDestroyActor(MultipleOwnershipPawn2);

		FinishStep();
	});

	// All workers set a reference to the MultipleOwnershipPawns in their world, to avoid code duplication.
	AddStep(
		TEXT("SpatialTestMultipleOwnershipAllWorkersSetReferences"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float DeltaTime) {
			TArray<AActor*> FoundPawns;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMultipleOwnershipPawn::StaticClass(), FoundPawns);

			// Make sure the correct number of MultipleOwnershipPawns is visible
			if (FoundPawns.Num() != 2)
			{
				return;
			}

			// Preemptively empty the Pawns array so that we don't get any issues with index-based addressing later on in the test
			MultipleOwnershipPawns.Empty();

			for (AActor* Pawn : FoundPawns)
			{
				AMultipleOwnershipPawn* MultipleOwnershipPawn = Cast<AMultipleOwnershipPawn>(Pawn);
				if (MultipleOwnershipPawn != nullptr)
				{
					MultipleOwnershipPawns.Add(MultipleOwnershipPawn);
				}
			}
			// We want to be sure that the pawns are in the correct order, otherwise the ReceivedRPCs number may be swapped causing the test
			// to fail
			MultipleOwnershipPawns.Sort([](AMultipleOwnershipPawn& LHS, AMultipleOwnershipPawn& RHS) -> bool {
				return LHS.GetActorLocation().Y < RHS.GetActorLocation().Y;
			});
			// Double checking that we have the correct number of MultipleOwnershipPawns, just to make sure that the casting above was
			// successful and no issues arise from that
			if (MultipleOwnershipPawns.Num() == 2)
			{
				FinishStep();
			}
		},
		5.0f);

	// Step definition for Client 1 to send a Server RPC which is used multiple times during the test. RPCs are expected to miss twice on
	// the first pass, then once, then 0 times..
	FSpatialFunctionalTestStepDefinition ClientSendRPCStepDefinition;
	ClientSendRPCStepDefinition.StepName = TEXT("ClientSendRPCsTest");
	ClientSendRPCStepDefinition.bIsNativeDefinition = true;
	ClientSendRPCStepDefinition.NativeStartEvent.BindLambda([this]() {
		for (AMultipleOwnershipPawn* MultipleOwnershipPawn : MultipleOwnershipPawns)
		{
			MultipleOwnershipPawn->ServerSendRPC();
		}

		FinishStep();
	});

	// Client 1 sends an RPC from both MultipleOwnershipPawns
	AddStepFromDefinition(ClientSendRPCStepDefinition, FWorkerDefinition::Client(1));

	// All workers check that both MultipleOwnershipPawns have received 0 RPCs, therefore that the RPCs were ignored since the
	// MultipleOwnershipPawns had no owner.
	AddStep(
		TEXT("SpatialTestMultipleOwnershipAllWorkersCheckCorrectRPCs"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(0, MultipleOwnershipPawns[0]->ReceivedRPCs, TEXT("Pawn 0 received correct number of RPCs"));
			RequireEqual_Int(0, MultipleOwnershipPawns[1]->ReceivedRPCs, TEXT("Pawn 1 received correct number of RPCs"));
			FinishStep();
		},
		10.0f);

	// The server makes Client 1's PlayerController possess the first MultipleOwnershipPawn.
	AddStep(TEXT("SpatialTestMultipleOwnershipPossessPawn1"), FWorkerDefinition::Server(1), nullptr, [this]() {
		APlayerController* PlayerController =
			Cast<APlayerController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
		PlayerController->Possess(MultipleOwnershipPawns[0]);

		FinishStep();
	});

	// Client 1 sends an RPC from both MultipleOwnershipPawns
	AddStepFromDefinition(ClientSendRPCStepDefinition, FWorkerDefinition::Client(1));

	auto PossessesCorrectPawns2 = [this]() {
		APlayerController* PlayerController =
			Cast<APlayerController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
		return PlayerController == nullptr || PlayerController->GetPawn() == MultipleOwnershipPawns[0];
	};
	// All workers check that the RPC sent from the first MultipleOwnershipPawn was correctly received, whilst the one sent from the second
	// MultipleOwnershipPawn was ignored.
	AddStep(
		TEXT("SpatialTestMultipleOwnershipAllWorkersCheckCorrectRPCs2"), FWorkerDefinition::AllWorkers, PossessesCorrectPawns2, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(1, MultipleOwnershipPawns[0]->ReceivedRPCs, TEXT("Pawn 0 received correct number of RPCs"));
			RequireEqual_Int(0, MultipleOwnershipPawns[1]->ReceivedRPCs, TEXT("Pawn 1 received correct number of RPCs"));
			FinishStep();
		},
		10.0f);

	// The server makes Client 1's PlayerController possess the second MultipleOwnershipPawn.
	AddStep(TEXT("SpatialTestMultipleOwnershipPossessPawn2"), FWorkerDefinition::Server(1), nullptr, [this]() {
		APlayerController* PlayerController =
			Cast<APlayerController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
		PlayerController->Possess(MultipleOwnershipPawns[1]);
		MultipleOwnershipPawns[0]->SetOwner(PlayerController);

		FinishStep();
	});

	// Client 1 sends an RPC from both MultipleOwnershipPawns
	AddStepFromDefinition(ClientSendRPCStepDefinition, FWorkerDefinition::Client(1));

	auto PossessesCorrectPawns3 = [this]() {
		APlayerController* PlayerController =
			Cast<APlayerController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
		return (PlayerController == nullptr)
			   || (PlayerController->GetPawn() == MultipleOwnershipPawns[1] && MultipleOwnershipPawns[0]->GetOwner() == PlayerController);
	};
	// All workers check that the RPCs sent from both MultipleOwnershipPawns were correctly received.
	AddStep(
		TEXT("SpatialTestMultipleOwnershipAllWorkersCheckCorrectRPCs3"), FWorkerDefinition::AllWorkers, PossessesCorrectPawns3, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(2, MultipleOwnershipPawns[0]->ReceivedRPCs, TEXT("Pawn 0 received correct number of RPCs"));
			RequireEqual_Int(1, MultipleOwnershipPawns[1]->ReceivedRPCs, TEXT("Pawn 1 received correct number of RPCs"));
			FinishStep();
		},
		10.0f);

	// The server makes Client 1's PlayerController unpossess the second MultipleOwnershipPawn.
	AddStep(TEXT("SpatialTestMultipleOwnershipUnpossesPawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
		APlayerController* PlayerController =
			Cast<APlayerController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
		PlayerController->UnPossess();
		MultipleOwnershipPawns[1]->SetOwner(PlayerController);
		FinishStep();
	});

	// Client 1 sends an RPC from both MultipleOwnershipPawns
	AddStepFromDefinition(ClientSendRPCStepDefinition, FWorkerDefinition::Client(1));

	auto PossessesCorrectPawns4 = [this]() {
		APlayerController* PlayerController =
			Cast<APlayerController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());
		return (PlayerController == nullptr)
			   || (PlayerController->GetPawn() == nullptr && MultipleOwnershipPawns[1]->GetOwner() == PlayerController
				   && MultipleOwnershipPawns[0]->GetOwner() == PlayerController);
	};
	// All workers check that the RPCs sent from both MultipleOwnershipPawns were correctly received.
	AddStep(
		TEXT("SpatialTestMultipleOwnershipAllWorkersCheckCorrectRPCs4"), FWorkerDefinition::AllWorkers, PossessesCorrectPawns4, nullptr,
		[this](float DeltaTime) {
			RequireEqual_Int(3, MultipleOwnershipPawns[0]->ReceivedRPCs, TEXT("Pawn 0 received correct number of RPCs"));
			RequireEqual_Int(2, MultipleOwnershipPawns[1]->ReceivedRPCs, TEXT("Pawn 1 received correct number of RPCs"));
			FinishStep();
		},
		10.0f);
}
