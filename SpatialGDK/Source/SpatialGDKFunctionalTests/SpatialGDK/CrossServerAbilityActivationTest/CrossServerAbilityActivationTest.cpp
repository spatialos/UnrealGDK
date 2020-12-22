// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerAbilityActivationTest.h"
#include "GA_IncrementSpyValue.h"
#include "GameplayTagContainer.h"

ACrossServerAbilityActivationTest::ACrossServerAbilityActivationTest()
{
	Author = "Tilman Schmidt";
	Description = TEXT("Tests that gameplay abilities can be activated across server boundaries.");
	DuplicateActivationCheckWaitTime = 2.0f;

	TargetActor = nullptr;
	StepTimer = -1.0f;
}

bool TargetActorReadyForActivation(ASpyValueGASTestActor* Target)
{
	return Target->GetLocalRole() == ENetRole::ROLE_SimulatedProxy && Target->GetCounter() == 0;
}

void ACrossServerAbilityActivationTest::PrepareTest()
{
	Super::PrepareTest();

	FName TargetActorTag = FName(TEXT("Target"));

	// Step 1 - Spawn Actor On Auth
	AddStep(TEXT("SERVER_1_Spawn"), FWorkerDefinition::Server(1), nullptr, [this, TargetActorTag]() {
		UWorld* World = GetWorld();
		TargetActor = World->SpawnActor<ASpyValueGASTestActor>({ 0.0f, 0.0f, 0.0f }, FRotator::ZeroRotator);
		AssertTrue(IsValid(TargetActor), TEXT("Target actor is valid."));
		AssertTrue(TargetActor->GetCounter() == 0, TEXT("Target actor spy counter starts at 0"));
		AddDebugTag(TargetActor, TargetActorTag);
		RegisterAutoDestroyActor(TargetActor);
		FinishStep();
	});

	AddStepSetTagDelegation(TargetActorTag, 2);

	// Step 2 - Trigger an ability by spec handle, cross-server, and confirm that it got activated
	AddStep(
		TEXT("SERVER_1_ActivateAbilityBySpecHandle"), FWorkerDefinition::Server(1),
		[this]() {
			return TargetActorReadyForActivation(TargetActor);
		},
		[this]() {
			UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
			AssertIsValid(ASC, TEXT("TargetActor has an ability system component."));

			TArray<FGameplayAbilitySpec*> FoundAbilitySpecs;
			ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(UGA_IncrementSpyValue::GetTriggerTag()),
																	 FoundAbilitySpecs);
			AssertEqual_Int(FoundAbilitySpecs.Num(), 1, TEXT("Target actor has one granted instance of the ability"));
			FGameplayAbilitySpecHandle AbilityHandle = FoundAbilitySpecs[0]->Handle;
			AssertTrue(AbilityHandle.IsValid(), TEXT("Activatable ability has a valid handle"));

			UE_LOG(LogTemp, Log, TEXT("Activating ability"));
			ASC->CrossServerTryActivateAbility(AbilityHandle);
		},
		[this](float DeltaTime) {
			bool bShouldFinishStep = WaitForActivationConfirmation(DeltaTime);
			if (bShouldFinishStep)
			{
				FinishStep();
			}
		});

	// Step 3 - Same as 2, but activate by class
	AddStep(
		TEXT("SERVER_1_ActivateAbilityByClass"), FWorkerDefinition::Server(1),
		[this]() {
			return TargetActorReadyForActivation(TargetActor);
		},
		[this]() {
			UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
			AssertIsValid(ASC, TEXT("TargetActor has an ability system component."));

			UE_LOG(LogTemp, Log, TEXT("Activating ability"));
			ASC->CrossServerTryActivateAbilityByClass(UGA_IncrementSpyValue::StaticClass());
		},
		[this](float DeltaTime) {
			bool bShouldFinishStep = WaitForActivationConfirmation(DeltaTime);
			if (bShouldFinishStep)
			{
				FinishStep();
			}
		});

	// Step 4 - Same as 2, but activate by tag
	AddStep(
		TEXT("SERVER_1_ActivateAbilityByTag"), FWorkerDefinition::Server(1),
		[this]() {
			return TargetActorReadyForActivation(TargetActor);
		},
		[this]() {
			UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
			AssertIsValid(ASC, TEXT("TargetActor has an ability system component."));

			UE_LOG(LogTemp, Log, TEXT("Activating ability"));
			ASC->CrossServerTryActivateAbilitiesByTag(FGameplayTagContainer(UGA_IncrementSpyValue::GetTriggerTag()));
		},
		[this](float DeltaTime) {
			bool bShouldFinishStep = WaitForActivationConfirmation(DeltaTime);
			if (bShouldFinishStep)
			{
				FinishStep();
			}
		});

	// Step 5 - Activate by event.
	AddStep(
		TEXT("SERVER_1_ActivateAbilityByEvent"), FWorkerDefinition::Server(1),
		[this]() {
			return TargetActorReadyForActivation(TargetActor);
		},
		[this]() {
			UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
			AssertIsValid(ASC, TEXT("TargetActor has an ability system component."));

			UE_LOG(LogTemp, Log, TEXT("Activating ability"));

			FGameplayEventData EventData;
			ASC->CrossServerHandleGameplayEvent(UGA_IncrementSpyValue::GetTriggerTag(), EventData);
		},
		[this](float DeltaTime) {
			bool bShouldFinishStep = WaitForActivationConfirmation(DeltaTime);
			if (bShouldFinishStep)
			{
				FinishStep();
			}
		});
}

bool ACrossServerAbilityActivationTest::WaitForActivationConfirmation(float DeltaTime)
{
	// StepTimer == -1.0f signals that we have not yet seen Counter be 1
	if (StepTimer != -1.0f)
	{
		StepTimer += DeltaTime;
	}

	int Counter = TargetActor->GetCounter();
	if (Counter == 0)
	{
		// The counter is allowed to be 0 while we are still waiting for the changed counter to replicated to us.
		// However, if we've seen Counter at 1 before (and as a result started the timer), if we see it back at 0,
		// it somehow got reset. This likely indicates a bug in the test implementation.
		if (StepTimer != -1.0f)
		{
			FinishTest(EFunctionalTestResult::Invalid,
					   TEXT("The spy actor counter was reset after being set to 1. This is probably a bug in this test implementation."));
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
			TargetActor->ResetCounter();
			return true;
		}
	}
	else
	{
		FinishTest(EFunctionalTestResult::Failed,
				   FString::Printf(TEXT("The ability was activated %d times. It should only be activated once."), Counter));
	}

	return false;
}
