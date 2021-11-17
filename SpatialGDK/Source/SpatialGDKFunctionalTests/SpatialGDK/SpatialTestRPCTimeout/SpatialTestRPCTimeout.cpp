// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRPCTimeout.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialTestRPCTimeoutGameMode.h"
#include "SpatialTestRPCTimeoutPlayerController.h"

/**
 * This test ensures that RPC calls with unresolved parameters are queued until their parameters are correctly resolved.
 * This test contains 1 Server and 2 Client workers running in multiple processes.
 *
 * The flow is as follows:
 * - Setup:
 *  - Launch at least one client in a separate process
 * - Test:
 *  - All clients launched outside of the editor process must ensure that the referenced material asset is not in memory at the start of the
 * test.
 *  - All clients launched outside of the editor process must successfully resolve the material asset before passing it into a client RPC
 * function.
 */

ASpatialTestRPCTimeout::ASpatialTestRPCTimeout()
	: Super()
{
	Author = "Iwan";
	Description = TEXT(
		"This test calls an RPC with an asset that was softly referenced and check that it will be asynchronously loaded with a timeout of "
		"0.");
}

void ASpatialTestRPCTimeout::CreateCustomContentForMap()
{
	GetWorldSettingsForMap()->DefaultGameMode = ASpatialTestRPCTimeoutGameMode::StaticClass();
}

void ASpatialTestRPCTimeout::PrepareTest()
{
	Super::PrepareTest();

	AddStep(
		TEXT("Check that the material was not initially loaded on non-editor clients"), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			Step1Timer += DeltaTime;

			if (!GIsEditor)
			{
				ACharacter* TestCharacter = Cast<ACharacter>(GetLocalFlowPawn());
				ASpatialTestRPCTimeoutPlayerController* TestController =
					Cast<ASpatialTestRPCTimeoutPlayerController>(TestCharacter->GetController());

				if (TestController && TestCharacter)
				{
					UMaterial* Material = TestController->SoftMaterialPtr.Get();

					AssertTrue(Material == nullptr, TEXT("The soft-pointed material found in memory at the start of the test."));

					// While the PlayerController waits for 5 seconds before loading manually the material, there is a small latency between
					// that timer and the start of this tick event. As a result, we need to give ~1 second tolerance for the test to pass
					// consistently.
					RequireTrue(Step1Timer > 4.f,
								TEXT("The soft-pointed material must not be in memory until synchronously loaded after 5 seconds"));

					FinishStep();
				}
			}
			else
			{
				FinishStep();
			}
		},
		5.0f);

	AddStep(
		TEXT("Check that the material is correctly loaded after about 5 seconds delay. We should wait "), FWorkerDefinition::AllClients,
		nullptr, nullptr,
		[this](float DeltaTime) {
			if (!GIsEditor)
			{
				ACharacter* TestCharacter = Cast<ACharacter>(GetLocalFlowPawn());
				ASpatialTestRPCTimeoutPlayerController* TestController =
					Cast<ASpatialTestRPCTimeoutPlayerController>(TestCharacter->GetController());

				RequireTrue(TestController->IsSuccessfullyResolved(),
							TEXT("The soft-pointed material is synchronously loaded into the non-editor process."));
				FinishStep();
			}
			else
			{
				FinishStep();
			}
		},
		5.0f);
}
