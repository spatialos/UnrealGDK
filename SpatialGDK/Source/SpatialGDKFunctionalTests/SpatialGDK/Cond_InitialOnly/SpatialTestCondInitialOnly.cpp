// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestCondInitialOnly.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTestFlowController.h"
#include "TestCondInitialOnlyPawn.h"

ASpatialTestCondInitialOnly::ASpatialTestCondInitialOnly()
	: Super()
{
	Author = "Ken.Yu";
	Description = TEXT("Test Cond_InitialOnly Schema Generation");
}

void ASpatialTestCondInitialOnly::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Create a TestPawn"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		ATestCondInitialOnlyPawn* TestPawn =
			GetWorld()->SpawnActor<ATestCondInitialOnlyPawn>(FVector(50.0f, 50.0f, 50.0f), FRotator::ZeroRotator, FActorSpawnParameters());
		RegisterAutoDestroyActor(TestPawn);
		FinishStep();
	});

	Wait();

	AddStep(TEXT("Change TestValue"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float DeltaTime) {
		if (ATestCondInitialOnlyPawn* Pawn = GetPawn())
		{
			Pawn->SetTestValue(99);
			LogStep(ELogVerbosity::Log, "SetTestValue:99");
		}
		FinishStep();
	});

	Wait();

	AddStep(TEXT("Check TestValue"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime) {
		if (ATestCondInitialOnlyPawn* Pawn = GetPawn())
		{
			AssertEqual_Int(Pawn->GetTestValue(), ATestCondInitialOnlyPawn::InitialValue, TEXT("TestValue"), Pawn);
		}
		FinishStep();
	});
}

void ASpatialTestCondInitialOnly::Wait()
{
	AddStep(TEXT("Wait"), FWorkerDefinition::AllServers, nullptr, nullptr, [this](float DeltaTime) {
		if (WaitTime > 1.0f)
		{
			WaitTime = 0.0f;
			FinishStep();
		}
		WaitTime += DeltaTime;
	});
}

ATestCondInitialOnlyPawn* ASpatialTestCondInitialOnly::GetPawn()
{
	return Cast<ATestCondInitialOnlyPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), ATestCondInitialOnlyPawn::StaticClass()));
}
