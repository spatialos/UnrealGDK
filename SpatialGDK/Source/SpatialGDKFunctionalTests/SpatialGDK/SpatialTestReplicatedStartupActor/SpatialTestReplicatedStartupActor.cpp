// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "SpatialTestReplicatedStartupActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "Kismet/GameplayStatics.h"
#include "ReplicatedActor.h"
#include "SpatialFunctionalTestFlowController.h"
#include "Net/UnrealNetwork.h"
#include "ReplicatedStartupActorPlayerController.h"


/**
 * This test automates the ReplicatedStartupActor gym. The gym was used for:
 * - QA workflows Test Replicated startup actor are correctly spawned on all clients
 * - To support QA test case "C1944 Replicated startup actors are correctly spawned on all clients"
 * NOTE: This test requires a specific Map, with a custom GameMode and PlayerController, trying to run this test on a different Map will make it fail.
 *
 * The flow is as follows:
 * - Setup:
 *  - Each client sets its reference to the replicated actor and sends a server RPC.
 * - Test:
 *  - Each client tests that the server has a valid reference to its replicated actor.
 */

ASpatialTestReplicatedStartupActor::ASpatialTestReplicatedStartupActor()
	: Super()
{
	Author = "Andrei";
	Description = TEXT("Test Replicated Startup Actor");

	bIsValidReference = false;
}

void ASpatialTestReplicatedStartupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestReplicatedStartupActor, bIsValidReference);
}

void ASpatialTestReplicatedStartupActor::BeginPlay()
{
	Super::BeginPlay();

	AddClientStep(TEXT("SpatialTestReplicatedStartupActorClientsSetup"), FWorkerDefinition::ALL_WORKERS_ID, nullptr, [this](ASpatialFunctionalTest* NetTest)
		{
			TArray<AActor*> ReplicatedActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedActor::StaticClass(), ReplicatedActors);

			checkf(ReplicatedActors.Num() == 1, TEXT("There should be exactly 1 replicated actor"));

			AReplicatedStartupActorPlayerController* PlayerController = Cast<AReplicatedStartupActorPlayerController>(GetLocalFlowController()->GetOwner());

			PlayerController->ClientToServerRPC(this, ReplicatedActors[0]);

			FinishStep();
		});

	AddClientStep(TEXT("SpatialTestReplicatedStarupActorClientsCheckStep"), FWorkerDefinition::ALL_WORKERS_ID, nullptr, nullptr, [this](ASpatialFunctionalTest* NetTest, float DeltaTime)
		{
			if (bIsValidReference)
			{
				AssertTrue(bIsValidReference, TEXT("The server has a valid reference to this client's replicated actor"));

				AReplicatedStartupActorPlayerController* PlayerController = Cast<AReplicatedStartupActorPlayerController>(GetLocalFlowController()->GetOwner());
				PlayerController->ResetBoolean(this);

				FinishStep();
			}
		}, 2.0);
}
