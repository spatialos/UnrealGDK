// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerAbilityActivationTest.h"
#include "GA_IncrementSpyValue.h"
#include "GameplayTagContainer.h"

/**
 * Tests that a gameplay ability can be activated on an actor from a non-auth server via the cross-server API.
 * Activation by ability spec, class, tag, and gameplay event are tested.
 * - Setup:
 *	- Two servers, number of clients doesn't matter.
 *	- One test actor is spawned, which has an Ability System Component (ASC) and the ability to be activated already granted to it.
 *	- The test actor is force-delegated to server 2.
 * - Test:
 *	- Server one activates the ability on the actor via it's ability spec handle.
 *	  Then it waits for confirmation that the ability got activated via a replicated property on the test actor which gets changed by the
 *	  ability's code. If confirmation does not arrive within the step timeout, or if a double activation is detected, the test is failed.
 *	- The above step is repeated for activation by class, gameplay tag and gameplay event.
 * - Cleanup:
 *	- The test actor is registered to be auto-destroyed by the test framework at test end.
 */

ACrossServerAbilityActivationTest::ACrossServerAbilityActivationTest()
{
	Author = TEXT("Tilman Schmidt");
	Description = TEXT("Tests that gameplay abilities can be activated across server boundaries.");
	DuplicateActivationCheckWaitTime = 2.0f;

	TargetActor = nullptr;
	bTimerStarted = false;
}

void ACrossServerAbilityActivationTest::PrepareTest()
{
	Super::PrepareTest();

	FName TargetActorTag = FName(TEXT("Target"));

	// Step 1 - Spawn Actor On Auth
	AddStep(TEXT("Spawn actor"), FWorkerDefinition::Server(1), nullptr, [this, TargetActorTag]() {
		UWorld* World = GetWorld();
		TargetActor = World->SpawnActor<ASpyValueGASTestActor>(FVector::ZeroVector, FRotator::ZeroRotator);
		AssertTrue(IsValid(TargetActor), TEXT("Target actor is valid."));
		AssertTrue(TargetActor->GetCounter() == 0, TEXT("Target actor spy counter starts at 0"));
		AddDebugTag(TargetActor, TargetActorTag);
		RegisterAutoDestroyActor(TargetActor);
		FinishStep();
	});

	AddStepSetTagDelegation(TargetActorTag, 2);

	auto WaitForTargetReadyForActivation = [this]() {
		return TargetActor->GetLocalRole() == ENetRole::ROLE_SimulatedProxy && TargetActor->GetCounter() == 0;
	};

	auto CheckActivationConfirmed = [this](float DeltaTime) {
		if (bTimerStarted)
		{
			StepTimer += DeltaTime;
		}

		int Counter = TargetActor->GetCounter();
		if (Counter == 0)
		{
			// The counter is allowed to be 0 while we are still waiting for the changed counter to replicated to us.
			// However, if we've seen Counter at 1 before (and as a result started the timer), if we see it back at 0,
			// it somehow got reset. This likely indicates a bug in the test implementation.
			if (bTimerStarted)
			{
				FinishTest(
					EFunctionalTestResult::Invalid,
					TEXT("The spy actor counter was reset after being set to 1. This is probably a bug in this test implementation."));
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
				StepTimer = 0.0f;
				bTimerStarted = false;
				TargetActor->ResetCounter();
				FinishStep();
			}
		}
		else
		{
			FinishTest(EFunctionalTestResult::Failed,
					   FString::Printf(TEXT("The ability was activated %d times. It should only be activated once."), Counter));
		}
	};

	// Step 2 - Trigger an ability by spec handle, cross-server, and confirm that it got activated
	AddStep(
		TEXT("Activate Ability by Spec Handle"), FWorkerDefinition::Server(1), WaitForTargetReadyForActivation,
		[this]() {
			UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
			AssertIsValid(ASC, TEXT("TargetActor has an ability system component."));

			TArray<FGameplayAbilitySpec*> FoundAbilitySpecs;
			ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(UGA_IncrementSpyValue::GetTriggerTag()),
																	 FoundAbilitySpecs);
			AssertEqual_Int(FoundAbilitySpecs.Num(), 1, TEXT("Target actor has one granted instance of the ability"));
			FGameplayAbilitySpecHandle AbilityHandle = FoundAbilitySpecs[0]->Handle;
			AssertTrue(AbilityHandle.IsValid(), TEXT("Activatable ability has a valid handle"));

			ASC->CrossServerTryActivateAbility(AbilityHandle);
		},
		CheckActivationConfirmed);

	// Step 3 - Same as 2, but activate by class
	AddStep(
		TEXT("Activate Ability by Class"), FWorkerDefinition::Server(1), WaitForTargetReadyForActivation,
		[this]() {
			UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
			AssertIsValid(ASC, TEXT("TargetActor has an ability system component."));

			ASC->CrossServerTryActivateAbilityByClass(UGA_IncrementSpyValue::StaticClass());
		},
		CheckActivationConfirmed);

	// Step 4 - Same as 2, but activate by tag
	AddStep(
		TEXT("Activate Ability by Tag"), FWorkerDefinition::Server(1), WaitForTargetReadyForActivation,
		[this]() {
			UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
			AssertIsValid(ASC, TEXT("TargetActor has an ability system component."));

			ASC->CrossServerTryActivateAbilitiesByTag(FGameplayTagContainer(UGA_IncrementSpyValue::GetTriggerTag()));
		},
		CheckActivationConfirmed);

	// Step 5 - Activate by event.
	AddStep(
		TEXT("Activate Ability by Event"), FWorkerDefinition::Server(1), WaitForTargetReadyForActivation,
		[this]() {
			UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
			AssertIsValid(ASC, TEXT("TargetActor has an ability system component."));

			FGameplayEventData EventData;
			ASC->CrossServerHandleGameplayEvent(UGA_IncrementSpyValue::GetTriggerTag(), EventData);
		},
		CheckActivationConfirmed);
}
