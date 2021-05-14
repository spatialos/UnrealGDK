// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestPlayerDisconnectTrigger.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerDisconnectController.h"
#include "SpatialConstants.h"

ASpatialTestPlayerDisconnectTrigger::ASpatialTestPlayerDisconnectTrigger()
{
	Author = "Victoria Bloom";
	Description = TEXT("Ensure players are cleaned up correctly when they disconnected by the SHUTDOWN_PREPARATION worker flag.");
}

void ASpatialTestPlayerDisconnectTrigger::PrepareTest()
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
			TEXT("Server1_TriggerShutdownPreparation1"), FWorkerDefinition::Server(1), nullptr,
			[this]() {
				// set up the request to set the worker flag in the standard runtime.
				LocalShutdownRequest = FHttpModule::Get().CreateRequest();
				LocalShutdownRequest->SetVerb(TEXT("PUT"));
				LocalShutdownRequest->SetURL(TEXT("http://localhost:5006/worker_flag/workers/UnrealWorker/flags/")
											 + SpatialConstants::SHUTDOWN_PREPARATION_WORKER_FLAG);

				LocalShutdownRequest->SetContentAsString(
					TEXT("ValueA")); // The value doesn't matter. It's just here to set something that we can change later, so we're sure we
									 // trigger another notification from spatial
				if (!LocalShutdownRequest->ProcessRequest())
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("Failed to start the request to set the worker flag."));
					return;
				}
			},
			[this](float DeltaTime) {
				switch (LocalShutdownRequest->GetStatus())
				{
				case EHttpRequestStatus::Processing:
					break;
				case EHttpRequestStatus::Succeeded:
					FinishStep();
					break;
				default:
					FinishTest(EFunctionalTestResult::Failed, TEXT("Request to set the worker flag failed or was never started."));
					break;
				}
			});

		AddStep(
			TEXT("AllServers_ChecksAfter"), FWorkerDefinition::AllServers, nullptr,
			[this]() {
				int32 ActualNumberOfClients = GetNumberOfClientWorkers();
				RequireEqual_Int(ActualNumberOfClients, 0, TEXT("Expected zero clients."));

				FinishStep();
			},
			nullptr, 5.0f);
	}
}
