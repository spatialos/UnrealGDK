// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRemotePossession.h"
#include "Containers/Array.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPossession.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestFlowController.h"
#include "TestPossessionPawn.h"
#include "TestPossessionPlayerController.h"

/**
 * This test tests client possession over pawns in other zones.
 *
 * The test includes two servers and two client workers. The client workers begin with a player controller and their default pawns,
 * which they initially possess. The flow is as follows:
 *  - Setup:
 *    - Two test pawn actors are spawned, one for each client, with an offset in the y direction for easy visualisation
 *    - The controllers for each client  possess the spawned test pawn actors
 *  - Test:
 *    - The clients  assert that the pawn they currently possess are test pawns
 *  - Cleanup:
 *    - The clients repossess their default pawns
 *    - The test pawns are destroyed
 */

ASpatialTestRemotePossession::ASpatialTestRemotePossession()
	: Super()
{
	Author = "Jay";
	Description = TEXT("Test Actor Remote Possession");
}

void ASpatialTestRemotePossession::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestRemotePossession, OriginalPawns);
}

void ASpatialTestRemotePossession::PrepareTest()
{
	Super::PrepareTest();

	static FVector start[3] = { FVector{ 30.0f, 30.0f, 20.0f }, FVector{ -30.0f, 30.0f, 20.0f }, FVector{ -30.0f, -30.0f, 20.0f } };
	AddStep(TEXT("First step"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float) {
		int i = 0;
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
			{
				continue;
			}
			ATestPossessionPawn* TestPawn =
				GetWorld()->SpawnActor<ATestPossessionPawn>(start[i % 3], FRotator::ZeroRotator, FActorSpawnParameters());
			OriginalPawns.Add(TestPawn);
			++i;
			RegisterAutoDestroyActor(TestPawn);

			AController* PlayerController = Cast<AController>(FlowController->GetOwner());
			USpatialPossession::RemotePossess(PlayerController, TestPawn);
		}
		FinishStep();
	});

	AddStep(TEXT("Tick Step"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		static float deltaTotal{ 0.f };
		deltaTotal += DeltaTime;
		if (deltaTotal > 3.f)
		{
			FinishStep();
		}
	});

	AddStep(TEXT("Check"), FWorkerDefinition::Server(2), nullptr, nullptr, [this](float) {
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
			{
				continue;
			}
			AController* PlayerController = Cast<AController>(FlowController->GetOwner());
			if (PlayerController->HasAuthority())
			{
				USpatialPossession::RemotePossess(PlayerController, OriginalPawns[0]);
			}
			//
		}

		FinishStep();
	});

	AddStep(TEXT("Check"), FWorkerDefinition::Server(3), nullptr, nullptr, [this](float) {
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Server)
			{
				continue;
			}
			ATestPossessionPlayerController* PlayerController = Cast<ATestPossessionPlayerController>(FlowController->GetOwner());
			ensure(PlayerController);
			if (PlayerController->HasAuthority())
			{
				USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetNetDriver());
				NetDriver->LockingPolicy->AcquireLock(PlayerController, TEXT("TestLock"));
				USpatialPossession::RemotePossess(PlayerController, OriginalPawns[0]);
				AssertValue_Int(PlayerController->OnPossessFailedCalled, EComparisonMethod::Equal_To, 1, TEXT("OnPossessionFailed Called"));
			}
			//
		}

		FinishStep();
	});
}
