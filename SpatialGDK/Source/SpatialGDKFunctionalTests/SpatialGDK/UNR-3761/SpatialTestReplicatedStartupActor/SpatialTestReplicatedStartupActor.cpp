// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestReplicatedStartupActor.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "ReplicatedStartupActorPlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/TestActors/ReplicatedTestActorBase.h"

/**
 * This test automates the ReplicatedStartupActor gym. The gym was used for:
 * - QA workflows Test Replicated startup actors are correctly spawned on all clients
 * - To support QA test case "C1944 Replicated startup actors are correctly spawned on all clients"
 * NOTE: This test requires a specific Map with a ReplicatedTestActorBase placed on the map and in the interest of the players and
 *		 a custom GameMode and PlayerController, trying to run this test on a different Map will make it fail.
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

	AddStep(
		TEXT("SpatialTestReplicatedStartupActorClientsSetup"), FWorkerDefinition::AllClients,
		[this](ASpatialFunctionalTest* NetTest) {
			// Make sure that the PlayerController has been set before trying to do anything with it, this might prevent Null Pointer
			// exceptions being thrown when UE ticks at a relatively slow rate
			AReplicatedStartupActorPlayerController* PlayerController =
				Cast<AReplicatedStartupActorPlayerController>(GetLocalFlowController()->GetOwner());
			return IsValid(PlayerController);
		},
		[this](ASpatialFunctionalTest* NetTest) {
			TArray<AActor*> ReplicatedActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AReplicatedTestActorBase::StaticClass(), ReplicatedActors);

			checkf(ReplicatedActors.Num() == 1, TEXT("There should be exactly 1 replicated actor"));

			AReplicatedStartupActorPlayerController* PlayerController =
				Cast<AReplicatedStartupActorPlayerController>(GetLocalFlowController()->GetOwner());

			PlayerController->ClientToServerRPC(this, ReplicatedActors[0]);

			FinishStep();
		});

	AddStep(
		TEXT("SpatialTestReplicatedStarupActorClientsCheckStep"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](ASpatialFunctionalTest* NetTest, float DeltaTime) {
			if (bIsValidReference)
			{
				AssertTrue(bIsValidReference, TEXT("The server has a valid reference to this client's replicated actor"));

				AReplicatedStartupActorPlayerController* PlayerController =
					Cast<AReplicatedStartupActorPlayerController>(GetLocalFlowController()->GetOwner());
				PlayerController->ResetBoolean(this);

				FinishStep();
			}
		},
		2.0);
}
