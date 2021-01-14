// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PredictedGameplayCuesTest.h"
#include "AbilitySystemGlobals.h"
#include "GC_SignalCueActivation.h"
#include "GameFramework/Controller.h"
#include "GameplayCueManager.h"
#include "GameplayCueSet.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"

/**
 * Tests that gameplay cue events correctly trigger on all clients when triggered by a predicted gameplay effect application.
 * - Setup:
 *	- One server, two clients.
 *	- The server spawns the test pawn, which has an Ability System Component (ASC) and the ability to be activated already granted to it.
 *	- The test pawn is possessed by client 1's player controller. This allows client 1 to activate predicted abilities on the pawn.
 *	- All workers wait until the test pawn has been replicated to them.
 * - Test:
 *	- Client 1 activates the ability on the pawn through a gameplay event with the "execute" tag.
 *	  Based on the tag, the activated ability applies an instant gameplay effect, which has the gameplay cue to be tested assigned to it.
 *	  This should trigger an "execute" event on the gameplay cue.
 *  - All clients wait until they have seen then gameplay cue receive an "execute" event exactly once.
 *  - Client 1 activates the ability with the "activate" tag.
 *	  When activated with this tag, the ability applies a gameplay effect with duration,
 *    which should cause "OnActive", "WhileActive" and "Remove" events on the gameplay cue.
 *	- All clients wait until they have seen the gameplay cue receive an "OnActive" event exactly once.
 * - Cleanup:
 *	- The original pawn is re-possessed by client 1's player controller.
 *	- The test pawn is registered to be auto-destroyed by the test framework at test end.
 */
APredictedGameplayCuesTest::APredictedGameplayCuesTest()
{
	Author = TEXT("Tilman Schmidt");
	Description =
		TEXT("Tests that gameplay cue events correctly trigger on all clients when triggered by a predicted gameplay effect application.");
	SetNumRequiredClients(2);
	DuplicateActivationCheckWaitTime = 2.0f;

	TestPawn = nullptr;
	bTimerStarted = false;
	StepTimer = 0.0f;
}

void APredictedGameplayCuesTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Register gameplay cue"), FWorkerDefinition::AllClients, nullptr, [this]() {
		// A code-only gameplay cue will not get automatically picked up by the gameplay cue manager
		// (by default it searches the game content folder for assets).
		// If a cue is not registered with the manager, cue events will never be routed to it.
		// That means we have to register the cue manually here on each client.
		FSoftObjectPath CuePath(TEXT("/Script/SpatialGDKFunctionalTests.GC_SignalCueActivation"));
		FGameplayCueReferencePair ExecuteCueRef(UGC_SignalCueActivation::GetExecuteTag(), CuePath);
		FGameplayCueReferencePair AddCueRef(UGC_SignalCueActivation::GetAddTag(), CuePath);
		UAbilitySystemGlobals::Get().GetGameplayCueManager()->GetRuntimeCueSet()->AddCues({ ExecuteCueRef, AddCueRef });

		FinishStep();
	});

	auto WaitForEventConfirmation = [this](int Counter, float DeltaTime) {
		if (bTimerStarted)
		{
			StepTimer += DeltaTime;
		}

		if (Counter == 0)
		{
			// The counter is allowed to be 0 while we are still waiting for the changed counter to replicated to us.
			// However, if we've seen Counter at 1 before (and as a result started the timer), if we see it back at 0,
			// it somehow got reset. This likely indicates a bug in the test implementation.
			if (bTimerStarted)
			{
				FinishTest(EFunctionalTestResult::Invalid, TEXT("The counter was reset to 0 after being set to 1."));
			}
		}
		else if (Counter == 1)
		{
			if (!bTimerStarted)
			{
				bTimerStarted = true;
			}
			else if (StepTimer >= DuplicateActivationCheckWaitTime)
			{
				bTimerStarted = false;
				StepTimer = 0.0f;
				FinishStep();
			}
		}
		else
		{
			FinishTest(EFunctionalTestResult::Failed,
					   FString::Printf(TEXT("The event was triggered %d times. It should only be triggered once."), Counter));
		}
	};

	AddStep(
		TEXT("Spawn Actor"), FWorkerDefinition::Server(1), nullptr,
		[this]() {
			UWorld* World = GetWorld();
			// The exact spawn location doesn't matter much. 10,10 should at least not be on the boundary between servers
			TestPawn = World->SpawnActor<ACuesGASTestPawn>({ 10.0f, 10.0f, 0.0f }, FRotator::ZeroRotator);
			AssertTrue(IsValid(TestPawn), TEXT("Target actor is valid."));
			AssertEqual_Int(TestPawn->GetOnActiveCounter(), 0, TEXT("OnActive counter should start at 0."));
			AssertEqual_Int(TestPawn->GetExecuteCounter(), 0, TEXT("Executed counter should start at 0."));

			// Set the target to be owned by client 1. An actor has to be client-owned to be able to run client-predicted gameplay abilities
			// on it.
			ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
			AController* PlayerController = Cast<AController>(FlowController->GetOwner());

			// Store the current pawn so we can re-possess it during cleanup
			PrevPawn = PlayerController->GetPawn();
			PlayerController->Possess(TestPawn);

			RegisterAutoDestroyActor(TestPawn);
		},
		[this](float DeltaTime) {
			RequireTrue(TestPawn->IsActorReady(), TEXT("Spawn Actor: Expect TestPawn to be ready."));
			FinishStep();
		});

	AddStep(TEXT("Wait to see actor"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float DeltaTime) {
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACuesGASTestPawn::StaticClass(), FoundActors);

		if (FoundActors.Num() == 1)
		{
			TestPawn = static_cast<ACuesGASTestPawn*>(FoundActors[0]);
			FinishStep();
		}
	});

	AddStep(TEXT("Activate ability to execute cue"), FWorkerDefinition::Client(1), nullptr, [this]() {
		UAbilitySystemComponent* ASC = TestPawn->GetAbilitySystemComponent();
		AssertIsValid(ASC, TEXT("TestPawn has an ability system component."));
		FGameplayEventData EventData;
		ASC->HandleGameplayEvent(UGC_SignalCueActivation::GetExecuteTag(), &EventData);

		FinishStep();
	});

	AddStep(TEXT("Check that cue executed on all clients"), FWorkerDefinition::AllClients, nullptr, nullptr,
			[this, WaitForEventConfirmation](float DeltaTime) {
				WaitForEventConfirmation(TestPawn->GetExecuteCounter(), DeltaTime);
			});

	AddStep(TEXT("Activate ability to add cue"), FWorkerDefinition::Client(1), nullptr, [this]() {
		UAbilitySystemComponent* ASC = TestPawn->GetAbilitySystemComponent();
		AssertIsValid(ASC, TEXT("TestPawn has an ability system component."));
		FGameplayEventData EventData;
		ASC->HandleGameplayEvent(UGC_SignalCueActivation::GetAddTag(), &EventData);

		FinishStep();
	});

	AddStep(TEXT("Check that cue was added on all clients"), FWorkerDefinition::AllClients, nullptr, nullptr,
			[this, WaitForEventConfirmation](float DeltaTime) {
				WaitForEventConfirmation(TestPawn->GetOnActiveCounter(), DeltaTime);
			});

	AddStep(TEXT("Re-possess previous pawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
		// Re-possess the original pawn to clean up after ourselves
		ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
		AController* PlayerController = Cast<AController>(FlowController->GetOwner());
		PlayerController->Possess(PrevPawn);

		FinishStep();
	});
}
