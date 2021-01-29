// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "GenerateTestMapsCommandlet.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKFunctionalTests/Public/SpatialCleanupConnectionTest.h" //didn't spot this during review, but I would argue this one is "SpatialGDK" as well
#include "SpatialGDKFunctionalTests/Public/SpatialTest1x2GridSmallInterestWorkerSettings.h"
#include "SpatialGDKFunctionalTests/Public/Test1x2WorkerSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/CrossServerAndClientOrchestrationTest/CrossServerAndClientOrchestrationTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyAndTombstoneTest/DormancyAndTombstoneTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/DormancyAndTombstoneTest/DormancyTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/RegisterAutoDestroyActorsTest/RegisterAutoDestroyActorsTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/RelevancyTest/RelevancyTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTestGameMode.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialAuthorityTest/SpatialAuthorityTestReplicatedActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialComponentTest/SpatialComponentTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialComponentTest/SpatialComponentTestActor.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialComponentTest/SpatialComponentTestReplicatedActor.h"
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
		500, 500, 1)); // Make the initial platform larger so things don't fall off for tests which use a large area (visibility test)

	AExponentialHeightFog* Fog =
		CastChecked<AExponentialHeightFog>(GEditor->AddActor(CurrentLevel, AExponentialHeightFog::StaticClass(), FTransform::Identity));

	APlayerStart* PlayerStart =
		CastChecked<APlayerStart>(GEditor->AddActor(CurrentLevel, APlayerStart::StaticClass(), FTransform(FVector(-500, 0, 100))));

	ASkyLight* SkyLight = CastChecked<ASkyLight>(GEditor->AddActor(CurrentLevel, ASkyLight::StaticClass(), FTransform::Identity));

	// Use current settings for all these generated maps as default.
	// This will probably change with UNR-4801
	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(CurrentWorld->GetWorldSettings());
	WorldSettings->TestingSettings.TestingMode = EMapTestingMode::UseCurrentSettings;
	WorldSettings->bEnableDebugInterface = true; // Setting to true for all test maps by default, TODO: consult Nic

	// TODO: Maybe figure out how to create folders to put the actors into (just visual if someone opens the map in the editor)
	// TODO: Maybe figure out how to set the default viewpoint when opening the map, so we don't start in the ground

	return CurrentWorld;
}

// This map is mostly for native-spatial compatibility tests, so things that work in native that should also work under our networking
// If you can, please add your test to this map, the more tests we have in a map, the more efficient it is to run them (no need for a new
// session / deployment for every test). That does mean that tests here should ideally create everything they need dynamically, and clean up
// everything after themselves, so other tests can run with a clean slate.
void UGenerateTestMapsCommandlet::CreateSpatialNetworkingMap()
{
	ULevel* CurrentLevel = CreateNewTestMapWorld()->GetCurrentLevel();

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
	// Unfortunately, the nature of some tests requires them to have actors placed in the level, to trigger some Unreal behavior
	GEditor->AddActor(CurrentLevel, ADormancyTestActor::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, AReplicatedVisibilityTestActor::StaticClass(), FTransform::Identity);

	FString PathToSave = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()
														   + TEXT("Maps/FunctionalTests/GeneratedMaps/CI_Fast/SpatialNetworkingMap.umap"));
	FEditorFileUtils::SaveLevel(CurrentLevel, PathToSave);
}

// This map is a simple 2-server-worker map, where both workers see everything in the world
void UGenerateTestMapsCommandlet::CreateSpatial2WorkerMap()
{
	UWorld* CurrentWorld = CreateNewTestMapWorld();
	ULevel* CurrentLevel = CurrentWorld->GetCurrentLevel();

	// Add the tests

	// The two orchestration tests are placed at opposite points around the origin to "guarantee" they will land on different workers so
	// that they can demonstrate they work in all situations
	GEditor->AddActor(CurrentLevel, ACrossServerAndClientOrchestrationTest::StaticClass(), FTransform(FVector(-250, 250, 0)));
	GEditor->AddActor(CurrentLevel, ACrossServerAndClientOrchestrationTest::StaticClass(), FTransform(FVector(250, -250, 0)));
	// This one lives in the TestGyms, TODO
	// GEditor->AddActor(CurrentLevel, APredictedGameplayCuesTest::StaticClass(), FTransform::Identity);
	// This one lives in the TestGyms, TODO
	// GEditor->AddActor(CurrentLevel, ACrossServerAbilityActivationTest::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARegisterAutoDestroyActorsTestPart1::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARegisterAutoDestroyActorsTestPart2::StaticClass(), FTransform::Identity);
	GEditor->AddActor(CurrentLevel, ARelevancyTest::StaticClass(), FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(CurrentWorld->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest1x2WorkerSettings::StaticClass());

	FString PathToSave = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectContentDir() + TEXT("Maps/FunctionalTests/GeneratedMaps/CI_Fast_Spatial_Only/Spatial2WorkerMap.umap"));
	FEditorFileUtils::SaveLevel(CurrentLevel, PathToSave);
}

// This map is custom-made for the SpatialAuthorityTest - it utilizes a gamemode override
void UGenerateTestMapsCommandlet::CreateSpatialAuthorityMap()
{
	UWorld* CurrentWorld = CreateNewTestMapWorld();
	ULevel* CurrentLevel = CurrentWorld->GetCurrentLevel();

	// Add the test
	ASpatialAuthorityTest* AuthTestActor = CastChecked<ASpatialAuthorityTest>(
		GEditor->AddActor(CurrentLevel, ASpatialAuthorityTest::StaticClass(), FTransform(FVector(-250, -250, 0))));

	// Add the helpers, as we need things placed in the level
	// PRComment: Hmmmmmmmm, these CastChecked with the static class defined right there are kind of annoying
	// I could probably create a quick templated function that does this...? Followed GEditor->AddActor because it's used elsewhere in the
	// engine, but seems like it doesn't have a templated version
	AuthTestActor->LevelActor = CastChecked<ASpatialAuthorityTestActor>(
		GEditor->AddActor(CurrentLevel, ASpatialAuthorityTestActor::StaticClass(), FTransform(FVector(-250, -250, 0))));
	AuthTestActor->LevelReplicatedActor = CastChecked<ASpatialAuthorityTestReplicatedActor>(
		GEditor->AddActor(CurrentLevel, ASpatialAuthorityTestReplicatedActor::StaticClass(), FTransform(FVector(-250, -250, 0))));
	AuthTestActor->LevelReplicatedActorOnBorder = CastChecked<ASpatialAuthorityTestReplicatedActor>(
		GEditor->AddActor(CurrentLevel, ASpatialAuthorityTestReplicatedActor::StaticClass(),
						  FTransform(FVector(0, 0, 0)))); // Says "on the border", but this map doesn't have multi-worker...?

	AWorldSettings* WorldSettings = CurrentWorld->GetWorldSettings();
	WorldSettings->DefaultGameMode = ASpatialAuthorityTestGameMode::StaticClass();

	FString PathToSave = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()
														   + TEXT("Maps/FunctionalTests/GeneratedMaps/CI_Fast/SpatialAuthority.umap"));
	FEditorFileUtils::SaveLevel(CurrentLevel, PathToSave);
}

// This map is a simple 2-server-worker map, where both workers see only a tiny bit (150 units) outside of their authoritative zone
// meta comment ^ not sure how useful it is to include the interest radius here, as this is basically duplicating information... could just
// say small radius and refer to the implementation for the actual value
void UGenerateTestMapsCommandlet::CreateSpatial2WorkerSmallInterestMap()
{
	UWorld* CurrentWorld = CreateNewTestMapWorld();
	ULevel* CurrentLevel = CurrentWorld->GetCurrentLevel();

	// Add the tests
	GEditor->AddActor(CurrentLevel, ASpatialCleanupConnectionTest::StaticClass(),
					  FTransform(FVector(-50, -50, 0))); // Seems like this position is required so that the LB plays nicely?

	// Quirk of the test. We need the player spawns on the same portion of the map as the test, so they are LBed together
	AActor** PlayerStart = CurrentLevel->Actors.FindByPredicate([](AActor* Actor) {
		return Actor->GetClass() == APlayerStart::StaticClass();
	});
	(*PlayerStart)->SetActorLocation(FVector(-50, -50, 100));

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(CurrentWorld->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(USpatialTest1x2GridSmallInterestWorkerSettings::StaticClass());

	FString PathToSave = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectContentDir() + TEXT("Maps/FunctionalTests/GeneratedMaps/CI_Fast_Spatial_Only/Spatial2WorkerSmallInterestMap.umap"));
	FEditorFileUtils::SaveLevel(CurrentLevel, PathToSave);
}

// This map is custom-made for the SpatialComponentTest - it utilizes a gamemode override
// Actually seems weird, I think this could be merged with some other map...
void UGenerateTestMapsCommandlet::CreateSpatialComponentMap()
{
	UWorld* CurrentWorld = CreateNewTestMapWorld();
	ULevel* CurrentLevel = CurrentWorld->GetCurrentLevel();

	// Add the test
	ASpatialComponentTest* CompTest = CastChecked<ASpatialComponentTest>(
		GEditor->AddActor(CurrentLevel, ASpatialComponentTest::StaticClass(), FTransform(FVector(-250, -250, 0))));

	// Add the helpers, as we need things placed in the level
	CompTest->LevelActor = CastChecked<ASpatialComponentTestActor>(
		GEditor->AddActor(CurrentLevel, ASpatialComponentTestActor::StaticClass(), FTransform(FVector(-250, -250, 0))));
	CompTest->LevelReplicatedActor = CastChecked<ASpatialComponentTestReplicatedActor>(
		GEditor->AddActor(CurrentLevel, ASpatialComponentTestReplicatedActor::StaticClass(), FTransform(FVector(-250, -250, 0))));

	// Quirk of the test. We need the player spawns on the same portion of the map as the test, so they are LBed together
	AActor** PlayerStart = CurrentLevel->Actors.FindByPredicate([](AActor* Actor) {
		return Actor->GetClass() == APlayerStart::StaticClass();
	});
	(*PlayerStart)->SetActorLocation(FVector(-500, -250, 100));

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(CurrentWorld->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest1x2WorkerSettings::StaticClass());
	WorldSettings->DefaultGameMode = ASpatialAuthorityTestGameMode::StaticClass();

	FString PathToSave = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()
														   + TEXT("Maps/FunctionalTests/GeneratedMaps/CI_Fast/SpatialComponentMap.umap"));
	FEditorFileUtils::SaveLevel(CurrentLevel, PathToSave);
}

int32 UGenerateTestMapsCommandlet::Main(const FString& CmdLineParams)
{
	UE_LOG(LogGenerateTestMapsCommandlet, Display, TEXT("Generate test maps commandlet started. Pray."));

	UE_LOG(LogGenerateTestMapsCommandlet, Display, TEXT("Creating the SpatialNetworkingMap."));
	CreateSpatialNetworkingMap();
	UE_LOG(LogGenerateTestMapsCommandlet, Display, TEXT("Creating the Spatial2WorkerMap."));
	CreateSpatial2WorkerMap();
	UE_LOG(LogGenerateTestMapsCommandlet, Display, TEXT("Creating the Spatial2WorkerSmallInterestMap."));
	CreateSpatial2WorkerSmallInterestMap();
	UE_LOG(LogGenerateTestMapsCommandlet, Display, TEXT("Creating the SpatialAuthorityMap."));
	CreateSpatialAuthorityMap();
	UE_LOG(LogGenerateTestMapsCommandlet, Display, TEXT("Creating the SpatialComponentMap."));
	CreateSpatialComponentMap();

	// Success
	return 0;
}

#endif // WITH_EDITOR
