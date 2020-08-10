// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "OwnerOnlyPropertyReplication.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestFlowController.h"

namespace
{
FString AssertStep(const FSpatialFunctionalTestStepDefinition& StepDefinition, const FString& Text)
{
	return FString::Printf(TEXT("[%s] %s"), *StepDefinition.StepName, *Text);
}
} // namespace

/**
 * This test tests replication of owner-only properties on an actor.
 *
 * The test includes a single server and two client workers. The client workers begin with a player controller and their default pawns,
 * which they initially possess. The flow is as follows:
 *  - Setup:
 *    - No setup required
 *  - Test:
 *    - The server creates an instance of a test pawn and changes the value of an owner-only property to 42
 *    - The client verifies that its locally replicated version still has the default value
 *    - The server makes one of the clients possess the test pawn
 *    - The now owning client verifies that the owner-only property has now been replicated and shows 42
 *    - The non-owning client verifies that it still has the default value
 *    - The server changes the owner-only property to 666
 *    - The now owning client verifies that the owner-only property has been replicated and shows 666
 *    - The non-owning client verifies that it still has the default value
 *  - Cleanup:
 *    - No cleanup required, as the actor is deleted as part of the test.
 */
AOwnerOnlyPropertyReplication::AOwnerOnlyPropertyReplication()
{
	Author = "Andreas";
	Description = TEXT("UNR-3066 OwnerOnly replication test");
}

void AOwnerOnlyPropertyReplication::BeginPlay()
{
	Super::BeginPlay();

	{ // Step 1 - Set TestIntProp to 42.
		AddStep(TEXT("ServerCreateActor"), FWorkerDefinition::Server(1), nullptr, [](ASpatialFunctionalTest* NetTest) {
			AOwnerOnlyPropertyReplication* Test = Cast<AOwnerOnlyPropertyReplication>(NetTest);
			Test->Pawn = Test->GetWorld()->SpawnActor<AOwnerOnlyTestPawn>(FVector::ZeroVector, FRotator::ZeroRotator);
			Test->Pawn->SetReplicates(true);
			Test->Pawn->TestInt = 42;
			Test->RegisterAutoDestroyActor(Test->Pawn);

			Test->FinishStep();
		});
	}
	{ // Step 2 - Check on client that TestInt didn't replicate.
		AddStep(
			TEXT("ClientNoReplicationBeforePossess"), FWorkerDefinition::AllClients,
			[](ASpatialFunctionalTest* NetTest) -> bool {
				AOwnerOnlyPropertyReplication* Test = Cast<AOwnerOnlyPropertyReplication>(NetTest);
				return IsValid(Test->Pawn);
			},
			[](ASpatialFunctionalTest* NetTest) {
				AOwnerOnlyPropertyReplication* Test = Cast<AOwnerOnlyPropertyReplication>(NetTest);
				const FSpatialFunctionalTestStepDefinition StepDefinition = Test->GetStepDefinition(Test->GetCurrentStepIndex());
				if (Test->Pawn)
				{
					Test->AssertEqual_Int(Test->Pawn->TestInt, 0, AssertStep(StepDefinition, TEXT("Pawn has default value")), Test);
				}

				Test->FinishStep();
			});
	}
	{ // Step 3 - Possess actor.
		AddStep(TEXT("ServerPossessActor"), FWorkerDefinition::Server(1), nullptr, [](ASpatialFunctionalTest* NetTest) {
			AOwnerOnlyPropertyReplication* Test = Cast<AOwnerOnlyPropertyReplication>(NetTest);
			if (Test->Pawn)
			{
				ASpatialFunctionalTestFlowController* FlowController = Test->GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
				APlayerController* PlayerController = Cast<APlayerController>(FlowController->GetOwner());

				Test->OriginalPawns.Add(TPair<AController*, APawn*>(PlayerController, PlayerController->GetPawn()));

				PlayerController->Possess(Test->Pawn);
			}

			Test->FinishStep();
		});
	}
	{ // Step 4 - Check on client that TestInt did replicate now on owning client.
		AddStep(
			TEXT("ClientCheckReplicationAfterPossess"), FWorkerDefinition::AllClients,
			[](ASpatialFunctionalTest* NetTest) -> bool {
				AOwnerOnlyPropertyReplication* Test = Cast<AOwnerOnlyPropertyReplication>(NetTest);
				return IsValid(Test->Pawn);
			},
			nullptr,
			[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
				AOwnerOnlyPropertyReplication* Test = Cast<AOwnerOnlyPropertyReplication>(NetTest);
				const FSpatialFunctionalTestStepDefinition StepDefinition = Test->GetStepDefinition(Test->GetCurrentStepIndex());
				if (Test->Pawn)
				{
					ASpatialFunctionalTestFlowController* FlowController = Test->GetLocalFlowController();
					if (FlowController->WorkerDefinition.Id == 1)
					{
						if (Test->Pawn->GetController() == FlowController->GetOwner() && Test->Pawn->TestInt == 42)
						{
							Test->AssertTrue(Test->Pawn->GetController() == FlowController->GetOwner(),
											 AssertStep(StepDefinition, TEXT("Client is in possession of pawn")));
							Test->AssertEqual_Int(Test->Pawn->TestInt, 42,
												  AssertStep(StepDefinition, TEXT("Pawn's TestInt was replicated to owning client")), Test);
							Test->FinishStep();
						}
					}
					else
					{
						Test->StepTimer += DeltaTime;
						if (Test->StepTimer >= 1.0f)
						{
							Test->StepTimer = 0.0f;
							Test->AssertTrue(Test->Pawn->GetController() != FlowController->GetOwner(),
											 AssertStep(StepDefinition, TEXT("Client is not in possession of pawn")));
							Test->AssertEqual_Int(
								Test->Pawn->TestInt, 0,
								AssertStep(StepDefinition, TEXT("Pawn's TestInt was not replicated to non-owning client")), Test);
							Test->FinishStep();
						}
					}
				}
			});
	}
	{ // Step 5 - Change value on server.
		AddStep(TEXT("ServerChangeValue"), FWorkerDefinition::Server(1), nullptr, [](ASpatialFunctionalTest* NetTest) {
			AOwnerOnlyPropertyReplication* Test = Cast<AOwnerOnlyPropertyReplication>(NetTest);
			if (Test->Pawn)
			{
				Test->Pawn->TestInt = 666;
			}

			Test->FinishStep();
		});
	}
	{ // Step 6 - Check that value was replicated on owning client.
		AddStep(TEXT("ClientCheckReplicationAfterChange"), FWorkerDefinition::AllClients, nullptr, nullptr,
				[](ASpatialFunctionalTest* NetTest, float DeltaTime) {
					AOwnerOnlyPropertyReplication* Test = Cast<AOwnerOnlyPropertyReplication>(NetTest);
					const FSpatialFunctionalTestStepDefinition StepDefinition = Test->GetStepDefinition(Test->GetCurrentStepIndex());

					if (Test->Pawn)
					{
						ASpatialFunctionalTestFlowController* FlowController = Test->GetLocalFlowController();
						if (FlowController->WorkerDefinition.Id == 1)
						{
							if (Test->Pawn->TestInt == 666)
							{
								Test->AssertEqual_Int(
									Test->Pawn->TestInt, 666,
									AssertStep(StepDefinition, TEXT("Pawn's TestInt was replicated to owning client after being changed")),
									Test);
								Test->FinishStep();
							}
						}
						else
						{
							Test->StepTimer += DeltaTime;
							if (Test->StepTimer >= 1.0f)
							{
								Test->StepTimer = 0.0f;
								Test->AssertEqual_Int(
									Test->Pawn->TestInt, 0,
									AssertStep(StepDefinition,
											   TEXT("Pawn's TestInt was not replicated to non-owning client after being changed")),
									Test);
								Test->FinishStep();
							}
						}
					}
				});
	}
	{ // Step 7 - Put back original Pawns
		AddStep(TEXT("ServerPossessOriginalPawns"), FWorkerDefinition::Server(1), nullptr, [](ASpatialFunctionalTest* NetTest) {
			AOwnerOnlyPropertyReplication* Test = Cast<AOwnerOnlyPropertyReplication>(NetTest);
			for (const auto& OriginalPawnPair : Test->OriginalPawns)
			{
				OriginalPawnPair.Key->Possess(OriginalPawnPair.Value);
			}
			Test->FinishStep();
		});
	}
}

void AOwnerOnlyPropertyReplication::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOwnerOnlyPropertyReplication, Pawn);
}
