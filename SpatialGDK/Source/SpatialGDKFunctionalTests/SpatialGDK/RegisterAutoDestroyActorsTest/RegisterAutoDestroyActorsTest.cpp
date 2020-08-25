// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "RegisterAutoDestroyActorsTest.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/Character.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "SpatialFunctionalTestFlowController.h"

ARegisterAutoDestroyActorsTestPart1::ARegisterAutoDestroyActorsTestPart1()
{
	Author = "Nuno";
	Description = TEXT("Part1: Verify that server spawned a character and that is is visible to the clients");
}

void ARegisterAutoDestroyActorsTestPart1::BeginPlay()
{
	Super::BeginPlay();
	{ // Step 1 - Spawn Actor On Auth
		AddStep(TEXT("SERVER_1_Spawn"), FWorkerDefinition::Server(1), nullptr, [this]() {
			UWorld* World = GetWorld();
			int NumVirtualWorkers = GetNumberOfServerWorkers();

			// spawn 1 per server worker
			// since the information about positioning of the virtual workers is currently hidden, will assume they are all around zero
			// and spawn them in that radius
			FVector SpawnPosition = FVector(200.0f, -200.0f, 0.0f);
			FRotator SpawnPositionRotator = FRotator(0.0f, 360.0f / NumVirtualWorkers, 0.0f);
			for (int i = 0; i != NumVirtualWorkers; ++i)
			{
				ACharacter* Character = World->SpawnActor<ACharacter>(SpawnPosition, FRotator::ZeroRotator);
				AssertTrue(IsValid(Character),
						   FString::Printf(TEXT("Spawned ACharacter %s in worker %s"), *GetNameSafe(Character),
										   *GetFlowController(ESpatialFunctionalTestWorkerType::Server, i + 1)->GetDisplayName()));
				SpawnPosition = SpawnPositionRotator.RotateVector(SpawnPosition);
			}

			FinishStep();
		});
	}

	{ // Step 2 - Check If Clients have it
		AddStep(
			TEXT("CLIENT_ALL_CheckActorsSpawned"), FWorkerDefinition::AllClients, nullptr, nullptr,
			[this](float DeltaTime) {
				int NumCharactersFound = 0;
				int NumCharactersExpected = GetNumberOfServerWorkers();
				UWorld* World = GetWorld();
				for (TActorIterator<ACharacter> It(World); It; ++It)
				{
					++NumCharactersFound;
				}

				if (NumCharactersFound == NumCharactersExpected)
				{
					FinishStep();
				}
			},
			5.0f);
	}

	{ // Step 3 - Destroy by all servers that have authority
		AddStep(
			TEXT("SERVER_ALL_RegisterAutoDestroyActors"), FWorkerDefinition::AllServers,
			[this]() -> bool {
				int NumCharactersFound = 0;
				int NumCharactersExpected = 1;
				UWorld* World = GetWorld();
				for (TActorIterator<ACharacter> It(World, ACharacter::StaticClass()); It; ++It)
				{
					if (It->HasAuthority())
					{
						++NumCharactersFound;
					}
				}

				return NumCharactersFound == NumCharactersExpected;
			},
			[this]() {
				UWorld* World = GetWorld();
				for (TActorIterator<ACharacter> It(World); It; ++It)
				{
					if (It->HasAuthority())
					{
						AssertTrue(IsValid(*It), FString::Printf(TEXT("Registering ACharacter for destruction: %s"), *GetNameSafe(*It)));
						RegisterAutoDestroyActor(*It);
					}
				}
				FinishStep();
			},
			nullptr, 5.0f);
	}
}

ARegisterAutoDestroyActorsTestPart2::ARegisterAutoDestroyActorsTestPart2()
{
	Author = "Nuno";
	Description = TEXT("Part2: Verify that the actors have been destroyed across all workers");
}

void ARegisterAutoDestroyActorsTestPart2::BeginPlay()
{
	Super::BeginPlay();
	{
		// Check nobody has characters
		FSpatialFunctionalTestStepDefinition StepDefinition;
		StepDefinition.bIsNativeDefinition = true;
		StepDefinition.TimeLimit = 0.0f;
		StepDefinition.NativeStartEvent.BindLambda([this]() {
			UWorld* World = GetWorld();
			TActorIterator<ACharacter> It(World);
			bool bHasCharacter = static_cast<bool>(It);
			AssertFalse(bHasCharacter, FString::Printf(TEXT("Cleanup of ACharacter successful, no ACharacter found by %s"),
													   *GetLocalFlowController()->GetDisplayName()));
			FinishStep();
		});

		AddStepFromDefinition(StepDefinition, FWorkerDefinition::AllWorkers);
	}
}
