// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "PredictedGameplayCuesTest.h"
#include "AbilitySystemGlobals.h"
#include "GC_SignalCueActivation.h"
#include "GameplayCueManager.h"
#include "GameplayCueSet.h"
#include "Kismet/GameplayStatics.h"
#include "SpatialFunctionalTestFlowController.h"

APredictedGameplayCuesTest::APredictedGameplayCuesTest()
{
	Author = "Tilman Schmidt";
	Description = TEXT("Tests that gameplay abilities can be activated across server boundaries.");
	DuplicateActivationCheckWaitTime = 2.0f;

	TargetActor = nullptr;
	StepTimer = -1.0f;
}

// bool TargetActorReadyForActivation(ASpyValueGASTestActor* Target)
//{
//	return Target->GetLocalRole() == ENetRole::ROLE_SimulatedProxy && Target->GetCounter() == 0;
//}

void APredictedGameplayCuesTest::PrepareTest()
{
	Super::PrepareTest();

	// FName TargetActorTag = FName(TEXT("Target"));

	// Step 1 - Spawn Actor
	AddStep(
		TEXT("Spawn Actor"), FWorkerDefinition::Server(1), nullptr,
		[this]() {
			UWorld* World = GetWorld();
			// The exact spawn location doesn't matter much. 10,10 should at least not be on the boundary between servers
			TargetActor = World->SpawnActor<ACuesGASTestActor>({ 10.0f, 10.0f, 0.0f }, FRotator::ZeroRotator);
			AssertTrue(IsValid(TargetActor), TEXT("Target actor is valid."));
			AssertEqual_Int(TargetActor->GetAddCounter(), 0, TEXT("Added counter should start at 0."));
			AssertEqual_Int(TargetActor->GetExecuteCounter(), 0, TEXT("Executed counter should start at 0."));

			ASpatialFunctionalTestFlowController* FlowController = GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1);
			// Set the target to be owned by client 1 so that it can do client->server RPCs as part of activating the predicted ability
			TargetActor->SetOwner(FlowController);
			TargetActor->GetAbilitySystemComponent()->RefreshAbilityActorInfo();
			// Also tell the ASC who the owner actor. This will find the player controller in the owner chain and store it
			// (see FGameplayAbilityActorInfo::InitFromActor), which gets checked before allowing predictive abilities to run
			// (see UAbilitySystemComponent::HasNetworkAuthorityToActivateTriggeredAbility)
			// TargetActor->GetAbilitySystemComponent()->InitAbilityActorInfo(TargetActor, TargetActor);
			// TargetActor->GetAbilitySystemComponent->RefreshAbilityActorInfo();
			// Make the target an autonomous proxy on the client. Predicted abilities are only allowed to run on clients if the ASC's avatar
			// actor is an autonomous proxy.
			TargetActor->SetAutonomousProxy(true);

			// AddDebugTag(TargetActor, TargetActorTag);
			RegisterAutoDestroyActor(TargetActor);

			FSoftObjectPath CuePath(TEXT("/Script/SpatialGDKFunctionalTests.GC_SignalCueActivation"));
			FGameplayCueReferencePair ExecuteCueRef(UGC_SignalCueActivation::GetExecuteTag(), CuePath);
			FGameplayCueReferencePair AddCueRef(UGC_SignalCueActivation::GetAddTag(), CuePath);
			UAbilitySystemGlobals::Get().GetGameplayCueManager()->GetRuntimeCueSet()->AddCues({ ExecuteCueRef, AddCueRef });
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
			UE_LOG(LogTemp, Log, TEXT("Found target actor."));
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

	AddStep(TEXT("Check that cue executed on all clients"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime) {
		if (WaitForActivationConfirmation(TargetActor->GetExecuteCounter(), DeltaTime))
		{
			FinishStep();
		}
	});

	AddStep(TEXT("Activate ability to execute cue"), FWorkerDefinition::Client(1), nullptr, [this]() {
		UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
		AssertIsValid(ASC, TEXT("TargetActor has an ability system component."));
		FGameplayEventData EventData;
		ASC->HandleGameplayEvent(UGC_SignalCueActivation::GetAddTag(), &EventData);

		// FinishStep();
	});
}

bool APredictedGameplayCuesTest::WaitForActivationConfirmation(int Counter, float DeltaTime)
{
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
			FinishTest(EFunctionalTestResult::Invalid, TEXT("The counter was reset after being set to 1."));
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
			return true;
		}
	}
	else
	{
		FinishTest(EFunctionalTestResult::Failed,
				   FString::Printf(TEXT("The cue was activated %d times. It should only be activated once."), Counter));
	}

	return false;
}
