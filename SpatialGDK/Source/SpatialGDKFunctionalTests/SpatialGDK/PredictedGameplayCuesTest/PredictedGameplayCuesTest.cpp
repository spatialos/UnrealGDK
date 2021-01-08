// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PredictedGameplayCuesTest.h"
#include "AbilitySystemGlobals.h"
#include "GC_SignalCueActivation.h"
#include "GameplayCueManager.h"
#include "GameplayCueSet.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"

/**
 * Tests that gameplay cue events correctly trigger on all clients when triggered by a predicted gameplay effect application.
 * - Setup:
 *	- Two servers, two clients.
 *	- One test actor is spawned, which has an Ability System Component (ASC) and the ability to be activated already granted to it.
 *	- The test actor is set to be owned by client 1, and configured such that the client will be able to activate a locally predicted
 *	  ability.
 * - Test:
 *	- Server one activates the ability on the actor via it's ability spec handle.
 *	  Then it waits for confirmation that the ability got activated via a replicated property on the test actor which gets changed by the
 *	  ability's code. If confirmation does not arrive within the step timeout, or if a double activation is detected, the test is failed.
 *	- The above step is repeated for activation by class, gameplay tag and gameplay event.
 * - Cleanup:
 *	- The test actor is registered to be auto-destroyed by the test framework at test end.
 */
APredictedGameplayCuesTest::APredictedGameplayCuesTest()
{
	Author = TEXT("Tilman Schmidt");
	Description =
		TEXT("Tests that gameplay cue events correctly trigger on all clients when triggered by a predicted gameplay effect application.");
	DuplicateActivationCheckWaitTime = 2.0f;

	TargetActor = nullptr;
	StepTimer = -1.0f;
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
		// StepTimer == -1.0f signals that we have not yet seen Counter be 1
		if (StepTimer != -1.0f)
		{
			StepTimer += DeltaTime;
		}

		if (Counter == 0)
		{
			// The counter is allowed to be 0 while we are still waiting for the changed counter to replicated to us.
			// However, if we've seen Counter at 1 before (and as a result started the timer), if we see it back at 0,
			// it somehow got reset. This likely indicates a bug in the test implementation.
			if (StepTimer != -1.0f)
			{
				FinishTest(EFunctionalTestResult::Invalid, TEXT("The counter was reset to 0 after being set to 1."));
			}
		}
		else if (Counter == 1)
		{
			if (StepTimer == -1.0f)
			{
				StepTimer = 0.0f;
			}
			else if (StepTimer >= DuplicateActivationCheckWaitTime)
			{
				StepTimer = -1.0f;
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
			TargetActor = World->SpawnActor<ACuesGASTestActor>({ 10.0f, 10.0f, 0.0f }, FRotator::ZeroRotator);
			AssertTrue(IsValid(TargetActor), TEXT("Target actor is valid."));
			AssertEqual_Int(TargetActor->GetOnActiveCounter(), 0, TEXT("OnActive counter should start at 0."));
			AssertEqual_Int(TargetActor->GetExecuteCounter(), 0, TEXT("Executed counter should start at 0."));

			// Set the target to be owned by client 1. An actor has to be client-owned to be able to run client-predicted gameplay abilities
			// on it.
			ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
			TargetActor->SetOwner(FlowController);
			TargetActor->GetAbilitySystemComponent()->RefreshAbilityActorInfo();

			RegisterAutoDestroyActor(TargetActor);
		},
		[this](float DeltaTime) {
			if (TargetActor->IsActorReady())
			{
				FinishStep();
			}
		});

	AddStep(TEXT("Wait to see actor"), FWorkerDefinition::AllWorkers, nullptr, nullptr, [this](float DeltaTime) {
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACuesGASTestActor::StaticClass(), FoundActors);

		if (FoundActors.Num() == 1)
		{
			TargetActor = static_cast<ACuesGASTestActor*>(FoundActors[0]);
			FinishStep();
		}
	});

	AddStep(TEXT("Activate ability to execute cue"), FWorkerDefinition::Client(1), nullptr, [this]() {
		UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
		AssertIsValid(ASC, TEXT("TargetActor has an ability system component."));
		FGameplayEventData EventData;
		ASC->HandleGameplayEvent(UGC_SignalCueActivation::GetExecuteTag(), &EventData);

		FinishStep();
	});

	AddStep(TEXT("Check that cue executed on all clients"), FWorkerDefinition::AllClients, nullptr, nullptr,
			[this, WaitForEventConfirmation](float DeltaTime) {
				WaitForEventConfirmation(TargetActor->GetExecuteCounter(), DeltaTime);
			});

	AddStep(TEXT("Activate ability to add cue"), FWorkerDefinition::Client(1), nullptr, [this]() {
		UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
		AssertIsValid(ASC, TEXT("TargetActor has an ability system component."));
		FGameplayEventData EventData;
		ASC->HandleGameplayEvent(UGC_SignalCueActivation::GetAddTag(), &EventData);

		FinishStep();
	});

	AddStep(TEXT("Check that cue was added on all clients"), FWorkerDefinition::AllClients, nullptr, nullptr,
			[this, WaitForEventConfirmation](float DeltaTime) {
				WaitForEventConfirmation(TargetActor->GetOnActiveCounter(), DeltaTime);
			});
}
