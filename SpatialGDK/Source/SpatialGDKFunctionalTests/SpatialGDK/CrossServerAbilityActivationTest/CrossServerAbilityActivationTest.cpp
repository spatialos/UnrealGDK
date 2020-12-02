// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerAbilityActivationTest.h"
#include "GA_IncrementSpyValue.h"

ACrossServerAbilityActivationTest::ACrossServerAbilityActivationTest()
{
	Author = "Tilman Schmidt";
	Description = TEXT("Tests that gameplay abilities can be activated across server boundaries.");

	TargetActor = nullptr;
}

void ACrossServerAbilityActivationTest::PrepareTest()
{
	Super::PrepareTest();

	FName TargetActorTag = FName(TEXT("Target"));

	{ // Step 1 - Spawn Actor On Auth
		AddStep(TEXT("SERVER_1_Spawn"), FWorkerDefinition::Server(1), nullptr, [this, TargetActorTag]() {
			UWorld* World = GetWorld();
			TargetActor = World->SpawnActor<ASpyValueGASTestActor>({ 0.0f, 0.0f, 0.0f }, FRotator::ZeroRotator);
			AssertTrue(IsValid(TargetActor), TEXT("Target actor is valid."));
			AddDebugTag(TargetActor, TargetActorTag);
			RegisterAutoDestroyActor(TargetActor);
			FinishStep();
		});
	}

	AddStepSetTagDelegation(TargetActorTag, 2);

	{ // Step 2 - Trigger an ability by class, cross-server
		AddStep(TEXT("SERVER_1_ActivateAbility"), FWorkerDefinition::Server(1), nullptr, [this]() {
			UAbilitySystemComponent* ASC = TargetActor->GetAbilitySystemComponent();
			AssertTrue(ASC != nullptr, TEXT("TargetActor has an ability system component."));

			UE_LOG(LogTemp, Log, TEXT("Activating ability"));
			ASC->CrossServerTryActivateAbilityByClass(UGA_IncrementSpyValue::StaticClass());
			// bool bDidActivate = ASC->TryActivateAbilityByClass(UGA_IncrementSpyValue::StaticClass());
			// UE_LOG(LogTemp, Log, TEXT("Did activate: %s"), bDidActivate ? TEXT("True") : TEXT("False"));
			FinishStep();
		});
	}
}
