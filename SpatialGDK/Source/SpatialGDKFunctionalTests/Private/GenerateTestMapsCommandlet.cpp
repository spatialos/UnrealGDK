// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateTestMapsCommandlet.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/CrossServerAndClientOrchestrationTest/CrossServerAndClientOrchestrationTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyAndTombstoneTest/DormancyAndTombstoneTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyAndTombstoneTest/DormancyTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/RegisterAutoDestroyActorsTest/RegisterAutoDestroyActorsTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPossession/SpatialTestPossession.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestPossession/SpatialTestRepossession.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestRepNotify/SpatialTestRepNotify.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestSingleServerDynamicComponents/SpatialTestSingleServerDynamicComponents.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3066/OwnerOnlyPropertyReplication.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/UNR-3157/RPCInInterfaceTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/VisibilityTest/ReplicatedVisibilityTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/VisibilityTest/VisibilityTest.h"
#include "Tests/AutomationEditorCommon.h"

#if WITH_EDITOR

DEFINE_LOG_CATEGORY(LogGenerateTestMapsCommandlet);

UGenerateTestMapsCommandlet::UGenerateTestMapsCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;

	ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	check(PlaneAsset.Succeeded());
	PlaneStaticMesh = PlaneAsset.Object;

	ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
	check(MaterialAsset.Succeeded());
	BasicShapeMaterial = MaterialAsset.Object;
}

UWorld* UGenerateTestMapsCommandlet::CreateNewTestMapWorld()
{
	UWorld* CurrentWorld = FAutomationEditorCommonUtils::CreateNewMap();
	ULevel* CurrentLevel = CurrentWorld->GetCurrentLevel();

	AStaticMeshActor* Plane =
		CastChecked<AStaticMeshActor>(GEditor->AddActor(CurrentLevel, AStaticMeshActor::StaticClass(), FTransform::Identity));
	Plane->GetStaticMeshComponent()->SetStaticMesh(PlaneStaticMesh);
	Plane->GetStaticMeshComponent()->SetMaterial(0, BasicShapeMaterial);
	Plane->SetActorScale3D(FVector(
		500.0, 500.0, 1.0)); // Make the initial platform larger so things don't fall off for tests which use a large area (visibility test)

	AExponentialHeightFog* Fog =
		CastChecked<AExponentialHeightFog>(GEditor->AddActor(CurrentLevel, AExponentialHeightFog::StaticClass(), FTransform::Identity));

	APlayerStart* PlayerStart =
		CastChecked<APlayerStart>(GEditor->AddActor(CurrentLevel, APlayerStart::StaticClass(), FTransform::Identity));
	PlayerStart->SetActorLocation(FVector(-250, 0, 100));

	ASkyLight* SkyLight = CastChecked<ASkyLight>(GEditor->AddActor(CurrentLevel, ASkyLight::StaticClass(), FTransform::Identity));

	// TODO: Maybe figure out how to create folders to put the actors into (just visual if someone opens the map in the editor)

	return CurrentWorld;
}

void UGenerateTestMapsCommandlet::CreateSpatialNetworkingMap()
{
	UWorld* CurrentWorld = CreateNewTestMapWorld();
	ULevel* CurrentLevel = CurrentWorld->GetCurrentLevel();

	// Add the tests
	GEditor->AddActor(CurrentLevel, ASpatialTestPossession::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ASpatialTestRepossession::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ASpatialTestRepNotify::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, AVisibilityTest::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARPCInInterfaceTest::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARegisterAutoDestroyActorsTestPart1::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARegisterAutoDestroyActorsTestPart2::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, AOwnerOnlyPropertyReplication::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ADormancyAndTombstoneTest::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ASpatialTestSingleServerDynamicComponents::StaticClass(), FTransform::Identity);

	// Add test helpers
	GEditor->AddActor(CurrentLevel, ADormancyTestActor::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, AReplicatedVisibilityTestActor::StaticClass(), FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(CurrentWorld->GetWorldSettings());
	WorldSettings->TestingSettings.TestingMode = EMapTestingMode::UseCurrentSettings;

	FString PathToSave = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()
														   + TEXT("Maps/FunctionalTests/GeneratedMaps/CI_Fast/SpatialNetworkingMap.umap"));
	FEditorFileUtils::SaveLevel(CurrentLevel, PathToSave);
}

int32 UGenerateTestMapsCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(LogGenerateTestMapsCommandlet, Display, TEXT("Generate test maps commandlet started. Pray."));

	CreateSpatialNetworkingMap();

	// Success
	return 0;
}

#endif // WITH_EDITOR
