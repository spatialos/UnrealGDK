// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRPCTimeoutPlayerController.h"

#include "Chaos/PhysicalMaterials.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

ASpatialTestRPCTimeoutPlayerController::ASpatialTestRPCTimeoutPlayerController()
{
	// Choose materials which belong to the Engine. This is in anticipation of the possibility of moving this test to the UnrealGDK plugin
	// in the future.

	const TCHAR* FailedMaterialPathString =
		TEXT("Material'/Engine/EngineDebugMaterials/VertexColorViewMode_RedOnly.VertexColorViewMode_RedOnly'");
	static ConstructorHelpers::FObjectFinder<UMaterial> FailedMaterialFinder(FailedMaterialPathString);
	FailedMaterialAsset = FailedMaterialFinder.Object;
	checkf(IsValid(FailedMaterialAsset), TEXT("Could not find failed material asset %ls"), FailedMaterialPathString);

	const TCHAR* SoftMaterialPathString =
		TEXT("Material'/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP_Mat.TutorialTPP_Mat'");
	const FSoftObjectPath SoftMaterialPath = FSoftObjectPath(SoftMaterialPathString);
	SoftMaterialPtr = TSoftObjectPtr<UMaterial>(SoftMaterialPath);
}

bool ASpatialTestRPCTimeoutPlayerController::IsSuccessfullyResolved()
{
	return bSuccessfullyResolved;
}

void ASpatialTestRPCTimeoutPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	CheckMaterialLoaded();

	// Delay set material, let CheckMaterialLoaded() check if it's already loaded in memory
	GetWorld()->GetTimerManager().SetTimer(MaterialSetDelay, this, &ASpatialTestRPCTimeoutPlayerController::SetMaterialAfterDelay, 2.f, false);
}

void ASpatialTestRPCTimeoutPlayerController::CheckMaterialLoaded_Implementation()
{
	GetWorld()->GetTimerManager().SetTimer(HasValidCharacterTimer, this, &ASpatialTestRPCTimeoutPlayerController::CheckValidCharacter, 0.001, false);
}

void ASpatialTestRPCTimeoutPlayerController::SetMaterialAfterDelay()
{
	UMaterial* PlayerMaterial = SoftMaterialPtr.LoadSynchronous();
	OnSetMaterial(PlayerMaterial);
}

void ASpatialTestRPCTimeoutPlayerController::CheckValidCharacter()
{
	if (ACharacter* TestCharacter = Cast<ACharacter>(GetPawn()))
	{
		if (UMaterial* Material = SoftMaterialPtr.Get())
		{
			FTransform Transform = FTransform::Identity;
			Transform.SetRotation(FRotator::ZeroRotator.Quaternion());

			UTextRenderComponent* TRC = Cast<UTextRenderComponent>(
				TestCharacter->AddComponentByClass(UTextRenderComponent::StaticClass(), false, Transform, false));

			if (TRC)
			{
				TRC->SetText(FText::FromString("ERROR : Material already loaded on client, test is invalid"));
				TRC->SetTextRenderColor(FColor::Red);
			}
		}
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(HasValidCharacterTimer, this, &ASpatialTestRPCTimeoutPlayerController::CheckValidCharacter, 0.001,
											   false);
	}
}

void ASpatialTestRPCTimeoutPlayerController::OnSetMaterial_Implementation(UMaterial* PlayerMaterial)
{
	if (ACharacter* TestCharacter = Cast<ACharacter>(GetPawn()))
	{
		if (PlayerMaterial)
		{
			TestCharacter->GetMesh()->SetMaterial(0, PlayerMaterial);
			bSuccessfullyResolved = true;
		}
		else
		{
			TestCharacter->GetMesh()->SetMaterial(0, FailedMaterialAsset);
			bSuccessfullyResolved = false;
		}
	}
}
