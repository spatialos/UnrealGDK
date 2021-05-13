// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestPlayerDisconnectReturnToMainMenu.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerDisconnectController.h"
#include "SpatialConstants.h"
#include "SpatialFunctionalTestFlowController.h"

ASpatialTestPlayerDisconnectReturnToMainMenu::ASpatialTestPlayerDisconnectReturnToMainMenu()
{
	Author = "Victoria Bloom";
	Description = TEXT("Ensure players are cleaned up correctly when they disconnected by the return to main menu.");
	TriggerEventWaitTime = 5.0f;
	StepTimer = 0.0f;
}

void ASpatialTestPlayerDisconnectReturnToMainMenu::PrepareTest()
{
	Super::PrepareTest();
	{
		AddStep(
			TEXT("AllServers_ChecksBefore"), FWorkerDefinition::AllServers, nullptr,
			[this]() {
				int32 ActualNumberOfClients = GetNumberOfClientWorkers();
				RequireEqual_Int(ActualNumberOfClients, 2, TEXT("Expected two clients."));

				TArray<AActor*> PlayerControllers;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerDisconnectController::StaticClass(), PlayerControllers);
				RequireEqual_Int(PlayerControllers.Num(), 2, TEXT("Expected two player controllers."));

				TArray<AActor*> PlayerCharacters;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), PlayerCharacters);
				RequireEqual_Int(PlayerCharacters.Num(), 2, TEXT("Expected two player characters."));

				FinishStep();
			},
			nullptr, 5.0f);

		AddStep(
			TEXT("Client1_ReturnToMainMenu"), FWorkerDefinition::Client(1), nullptr,
			[this]() {

				APlayerDisconnectController* LocalPlayerController =
				Cast<APlayerDisconnectController> (UGameplayStatics::GetPlayerController(GetWorld(), 0));

				LocalPlayerController->ReturnToMainMenu();

				FinishStep();
			});

		// Need this additional step after client returns to main menu to deregister their flowcontroller from the server
		// If the client itself deregisters it's own flow controller it can never send the FinishStep command and will fail the step.
		AddStep(
			TEXT("AllServers_RemoveFlowControllerForClient1"), FWorkerDefinition::AllServers, nullptr,
			[this]() {
				TArray<AActor*> FoundActors;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpatialFunctionalTestFlowController::StaticClass(), FoundActors);
				for (AActor* Actor : FoundActors)
				{
					if (ASpatialFunctionalTestFlowController* FlowController = Cast<ASpatialFunctionalTestFlowController>(Actor))
					{
						if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client
							&& FlowController->WorkerDefinition.Id == 1)
						{
							FlowController->DeregisterFlowController();
						}
					}
				}

				FinishStep();
			},
			nullptr, 5.0f);

		AddStep(
			TEXT("AllServers_ChecksAfter"), FWorkerDefinition::AllServers, nullptr,
			[this]() {
				int32 ActualNumberOfClients = GetNumberOfClientWorkers();
				RequireEqual_Int(ActualNumberOfClients, 1, TEXT("Expected one client."));

				// TArray<AActor*> PlayerControllers;
				// UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerDisconnectController::StaticClass(), PlayerControllers);
				// RequireEqual_Int(PlayerControllers.Num(), 1, TEXT("Expected one player controllers."));

				/*TArray<AActor*> PlayerCharacters;
				UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), PlayerCharacters);
				RequireEqual_Int(PlayerCharacters.Num(), 1, TEXT("Expected one player characters."));*/

				FinishStep();
			},
			nullptr, 5.0f);
	}
}
