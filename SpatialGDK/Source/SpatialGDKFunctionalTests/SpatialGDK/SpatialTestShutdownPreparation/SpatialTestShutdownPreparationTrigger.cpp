// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestShutdownPreparationTrigger.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "SpatialConstants.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialFunctionalTestLBDelegationInterface.h"
#include "TestPrepareShutdownListener.h"

ASpatialTestShutdownPreparationTrigger::ASpatialTestShutdownPreparationTrigger()
{
	Author = "Tilman Schmidt";
	Description = TEXT("Trigger shutdown preparation via worker flags and make sure callbacks get called in C++ and Blueprints");
	StepTimer = 0.0f;
	LocalListener = nullptr;

	// set up the request to set the worker flag in the standard runtime.
	// Being lazy here and constructing one request on all workers, even though it will only be used on server worker 1
	LocalShutdownRequest = FHttpModule::Get().CreateRequest();
	LocalShutdownRequest->SetVerb(TEXT("PUT"));
	LocalShutdownRequest->SetURL(TEXT("http://localhost:5006/worker_flag/workers/UnrealWorker/flags/")
								 + SpatialConstants::SHUTDOWN_PREPARATION_WORKER_FLAG);
	LocalShutdownRequest->SetContentAsString(TEXT(""));
}

void ASpatialTestShutdownPreparationTrigger::BeginPlay()
{
	Super::BeginPlay();
	{ // Step 1 - Test print on all workers
		AddStep(TEXT("AllWorkers_SetupListener"), FWorkerDefinition::AllWorkers, nullptr, [this]() {
			UWorld* World = GetWorld();

			// Spawn a non-replicated actor that will listen for the shutdown event.
			// Using a non-replicated actor since this is the easiest way to make sure that every worker has exactly one instance of it.
			LocalListener =
				World->SpawnActor<ATestPrepareShutdownListener>(PrepareShutdownListenerClass, FVector::ZeroVector, FRotator::ZeroRotator);
			AssertTrue(IsValid(LocalListener), TEXT("Listener actor is valid."));
			RegisterAutoDestroyActor(LocalListener);

			LocalListener->RegisterCallback();
			if (LocalListener->NativePrepareShutdownEventCount != 0 || LocalListener->BlueprintPrepareShutdownEventCount != 0)
			{
				UE_LOG(LogTemp, Log, TEXT("Failing test due to event counts starting out wrong. native: %d, blueprint: %d"),
					   LocalListener->NativePrepareShutdownEventCount, LocalListener->BlueprintPrepareShutdownEventCount);
				FinishTest(EFunctionalTestResult::Failed, TEXT("Number of triggered events should start out at 0"));
				return;
			}

			FinishStep();
		});

		AddStep(
			TEXT("Server1_TriggerShutdownPreparation1"), FWorkerDefinition::Server(1), nullptr,
			[this]() {
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

		AddStep(TEXT("AllServers_CheckEventHasTriggered"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float DeltaTime) {
			// On servers, we expect the event to have been triggered
			if (LocalListener->NativePrepareShutdownEventCount == 1 && LocalListener->BlueprintPrepareShutdownEventCount == 1)
			{
				FinishStep();
				return;
			}
			// If the count is 0, we might not have received the event yet. We will keep checking by ticking this function.
		});

		AddStep(TEXT("AllClients_CheckEventHasNotTriggered"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime) {
			// On clients, the event should not be triggered
			if (LocalListener->NativePrepareShutdownEventCount != 0 || LocalListener->BlueprintPrepareShutdownEventCount != 0)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("The prepare shutdown event was received on a client"));
				return;
			}

			// The callback may take some time to be called on workers after being triggered. So we should wait a while before claiming that
			// it hasn't been called on a client.
			StepTimer += DeltaTime;
			if (StepTimer > EventWaitTime)
			{
				FinishStep();
				StepTimer = 0.0f;
			}
		});

		AddStep(
			TEXT("Server1_TriggerShutdownPreparation2"), FWorkerDefinition::Server(1), nullptr,
			[this]() {
				LocalShutdownRequest->SetContentAsString(TEXT("ValueB")); // again, the value doesn't matter
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

		AddStep(TEXT("AllServers_CheckEventHasTriggeredOnce"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float DeltaTime) {
			if (LocalListener->NativePrepareShutdownEventCount != 1 || LocalListener->BlueprintPrepareShutdownEventCount != 1)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("The prepare shutdown event has been received more than once."));
				return;
			}

			StepTimer += DeltaTime;
			if (StepTimer > EventWaitTime)
			{
				FinishStep();
				StepTimer = 0.0f;
			}
		});

		AddStep(TEXT("AllClients_CheckEventStillHasNotTriggered"), FWorkerDefinition::AllClients, nullptr, nullptr,
				[this](float DeltaTime) {
					// On clients, the event should not be triggered
					if (LocalListener->NativePrepareShutdownEventCount != 0 || LocalListener->BlueprintPrepareShutdownEventCount != 0)
					{
						FinishTest(EFunctionalTestResult::Failed, TEXT("The prepare shutdown event was received on a client"));
						return;
					}

					// The callback may take some time to be called on workers after being triggered. So we should wait a while before
					// claiming that it hasn't been called on a client.
					StepTimer += DeltaTime;
					if (StepTimer > EventWaitTime)
					{
						FinishStep();
						StepTimer = 0.0f;
					}
				});
	}
}
