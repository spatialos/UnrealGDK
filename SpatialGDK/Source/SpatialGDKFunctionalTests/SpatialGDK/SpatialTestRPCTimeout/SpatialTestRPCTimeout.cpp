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
			if (!GIsEditor)
			{
				ACharacter* TestCharacter = Cast<ACharacter>(GetLocalFlowPawn());
				ASpatialTestRPCTimeoutPlayerController* TestController = Cast<ASpatialTestRPCTimeoutPlayerController>(TestCharacter->GetController());

				if (TestController && TestCharacter)
				{
					UMaterial* Material = TestController->SoftMaterialPtr.Get();

					RequireTrue(Material == nullptr, TEXT("The soft-pointed material must not be in memory at the start of the test."));
					UE_LOG(LogTemp, Display, TEXT("A"));

					FinishStep();
				}
			}
			else
			{
				FinishStep();
			}
		},
		2.0f);

	AddStep(
		TEXT("Check that the material is correctly loaded after about 2 seconds delay. We should wait "), FWorkerDefinition::AllClients,
		nullptr, nullptr,
		[this](float DeltaTime) {
			if (!GIsEditor)
			{
				ACharacter* TestCharacter = Cast<ACharacter>(GetLocalFlowPawn());
				ASpatialTestRPCTimeoutPlayerController* TestController = Cast<ASpatialTestRPCTimeoutPlayerController>(TestCharacter->GetController());

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
