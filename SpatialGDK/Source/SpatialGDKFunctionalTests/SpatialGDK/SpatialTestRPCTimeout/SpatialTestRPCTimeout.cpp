// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRPCTimeout.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "SpatialFunctionalTestFlowController.h"
#include "RPCTimeoutPlayerController.h"
#include "RPCTimeoutGameMode.h"


ASpatialTestRPCTimeout::ASpatialTestRPCTimeout()
	: Super()
{
	Author = "Iwan";
	Description = TEXT("");
}

void ASpatialTestRPCTimeout::CreateCustomContentForMap()
{
	GetWorldSettingsForMap()->DefaultGameMode = ARPCTimeoutGameMode::StaticClass();
}

void ASpatialTestRPCTimeout::PrepareTest()
{
	Super::PrepareTest();


	AddStep(
		TEXT("Check that the material was not initially loaded on non-editor clients"), FWorkerDefinition::AllClients, nullptr,
		[this]() {
			if (!GIsEditor)
			{
				ACharacter* TestCharacter = Cast<ACharacter>(GetLocalFlowPawn());
				ARPCTimeoutPlayerController* TestController = Cast<ARPCTimeoutPlayerController>(TestCharacter->GetController());

				if (TestController && TestCharacter)
				{
					UMaterial* Material = TestController->SoftMaterialPtr.Get();

					RequireTrue(Material == nullptr, TEXT("The soft-pointed material must not be in memory at the start of the test."));

					FinishStep();
				}
			}
			else
			{
				FinishStep();
			}
		},
		nullptr, 5.0f);


	AddStep(
		TEXT("Check that the material is correctly loaded after about 3 seconds delay. This should be done synchronously bu the controller."), FWorkerDefinition::AllClients, nullptr, nullptr,
		[this](float DeltaTime) {
			if (!GIsEditor)
			{
				ACharacter* TestCharacter = Cast<ACharacter>(GetLocalFlowPawn());
				ARPCTimeoutPlayerController* TestController = Cast<ARPCTimeoutPlayerController>(TestCharacter->GetController());

				RequireTrue(TestController->SoftMaterialPtr.Get() != nullptr, TEXT("The soft-pointed material is synchronously loaded into the non-editor process."));
				FinishStep();
			}
			else
			{
				FinishStep();
			}
		}, 5.0f);
}
