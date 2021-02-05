// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/GeneratedTestMap.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/ConstructorHelpers.h"

UGeneratedTestMap::UGeneratedTestMap()
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	check(PlaneAsset.Succeeded());
	PlaneStaticMesh = PlaneAsset.Object;

	ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
	check(MaterialAsset.Succeeded());
	BasicShapeMaterial = MaterialAsset.Object;

	MapCategory = CI_FAST; // We will set the default to CI_FAST, even if almost every map will override this anyway
						   // Just in case we can make a newly written map be immediately run in CI when the PR is still in development, so
						   // that the developer gets feedback about whether their new map and presumably new test passes in CI
}

void UGeneratedTestMap::GenerateMap()
{
	GenerateBaseMap();
	CreateCustomContentForMap();
	SaveMap();
}

FString UGeneratedTestMap::GetGeneratedMapFolder()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + TEXT("Intermediate/Maps/"));
}

void UGeneratedTestMap::GenerateBaseMap()
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

	World = CurrentWorld;
}

void UGeneratedTestMap::SaveMap()
{
	FEditorFileUtils::SaveLevel(World->GetCurrentLevel(), GetPathToSaveTheMap());
}

FString UGeneratedTestMap::GetPathToSaveTheMap()
{
	// These names need to match the CI recognized ones (setup-build-test.ps1 in the GDK currently)
	FString DirName;
	switch (MapCategory)
	{
	case CI_FAST:
		DirName = TEXT("CI_Fast/");
		break;
	case CI_FAST_SPATIAL_ONLY:
		DirName = TEXT("CI_Fast_Spatial_Only/");
		break;
	case CI_SLOW:
		DirName = TEXT("CI_Slow/");
		break;
	case CI_SLOW_SPATIAL_ONLY:
		DirName = TEXT("CI_Slow_Spatial_Only/");
		break;
	case NO_CI:
		DirName = TEXT("");
		break;
	default:
		checkNoEntry();
	}

	return FString::Printf(TEXT("%s%s%s.umap"), *GetGeneratedMapFolder(), *DirName, *MapName);
}
