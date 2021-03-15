// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/GeneratedTestMap.h"
#include "Core/Public/Misc/FileHelper.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "FileHelpers.h"
#include "GameFramework/PlayerStart.h"
#include "SpatialGDKEditor/Public/SpatialTestSettings.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/ConstructorHelpers.h"
#include <Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>


UGeneratedTestMap::UGeneratedTestMap()
	: bIsValidForGeneration(false)
{
	ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
	check(PlaneAsset.Succeeded());
	PlaneStaticMesh = PlaneAsset.Object;

	ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	check(MaterialAsset.Succeeded());
	BasicShapeMaterial = MaterialAsset.Object;
}

UGeneratedTestMap::UGeneratedTestMap(EMapCategory InMapCategory, FString InMapName)
	: UGeneratedTestMap()
{
	MapCategory = InMapCategory;
	MapName = InMapName;
	bIsValidForGeneration = true;
}

AActor* UGeneratedTestMap::AddActorToLevel(ULevel* Level, UClass* Class, const FTransform& Transform)
{
	return GEditor->AddActor(Level, Class, Transform);
}


bool UGeneratedTestMap::GenerateMap()
{
	checkf(bIsValidForGeneration, TEXT("This test map object is not valid for map generation, please use the UGeneratedTestMap constructor "
									   "with arguments when deriving from the base UGeneratedTestMap."));
	GenerateBaseMap();
	CreateCustomContentForMap();
	return SaveMap();
}

bool UGeneratedTestMap::GenerateCustomConfig()
{
	FString CustomConfigContent = CustomConfigString;

	// If specified, append the setting to set the number of clients.
	if (NumberOfClients.IsSet())
	{
		if (!CustomConfigString.IsEmpty())
		{
			CustomConfigContent += LINE_TERMINATOR;
		}
		// clang-format off
		CustomConfigContent += FString::Printf(TEXT("[/Script/UnrealEd.LevelEditorPlaySettings]") LINE_TERMINATOR
											   TEXT("PlayNumberOfClients=%d"), NumberOfClients.GetValue());
		// clang-format on
	}

	if (CustomConfigContent.IsEmpty())
	{
		// Only create a custom config file if we have something meaningful to write so we don't pollute the file system too much
		return true;
	}

	const FString OverrideSettingsFilename =
		FSpatialTestSettings::GeneratedOverrideSettingsBaseFilename + MapName + FSpatialTestSettings::OverrideSettingsFileExtension;

	return FFileHelper::SaveStringToFile(CustomConfigContent, *OverrideSettingsFilename);
}

FString UGeneratedTestMap::GetGeneratedMapFolder()
{
	return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + TEXT("Intermediate/Maps/"));
}

void UGeneratedTestMap::GenerateBaseMap()
{
	World = FAutomationEditorCommonUtils::CreateNewMap();
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Lights and fog are just for visual effects, strictly not needed for running tests, but it makes it a bit nicer to look at tests while
	// they are running.
	AExponentialHeightFog* Fog = AddActorToLevel<AExponentialHeightFog>(CurrentLevel, FTransform::Identity);
	ASkyLight* SkyLight = AddActorToLevel<ASkyLight>(CurrentLevel, FTransform::Identity);

	// On the other hand, the plane and the player start are needed for tests and they rely on various properties of them (plane catches
	// things so they don't fall, player start controls not just the viewport, but is also used for certain tests to see how the spawned
	// player behaves under LB conditions).
	AStaticMeshActor* Plane = AddActorToLevel<AStaticMeshActor>(CurrentLevel, FTransform::Identity);
	Plane->GetStaticMeshComponent()->SetStaticMesh(PlaneStaticMesh);
	Plane->GetStaticMeshComponent()->SetMaterial(0, BasicShapeMaterial);
	// Make the initial platform much much larger so things don't fall off for tests which use a large area (visibility test)
	Plane->SetActorScale3D(FVector(10000, 10000, 1));

	// Default player start location is chosen so that players spawn on server 1 by default.
	// Individual test maps can change this if necessary.
	FTransform PlayerTransform;
	FVector PlayerTranslation = FVector(-500, -500, 200);
 	PlayerTransform.SetTranslation(PlayerTranslation);
	FRotator RotatorToOrigin = UKismetMathLibrary::MakeRotFromX(-PlayerTranslation);
	PlayerTransform.SetRotation(RotatorToOrigin.Quaternion());
	APlayerStart* PlayerStart = AddActorToLevel<APlayerStart>(CurrentLevel, PlayerTransform);
}

bool UGeneratedTestMap::SaveMap()
{
	const bool bSuccess = FEditorFileUtils::SaveLevel(World->GetCurrentLevel(), GetPathToSaveTheMap());
	UE_CLOG(!bSuccess, LogGenerateTestMapsCommandlet, Error, TEXT("Failed to save the map %s."), *GetMapName());
	return bSuccess;
}

FString UGeneratedTestMap::GetPathToSaveTheMap()
{
	// These names need to match the CI recognized ones (setup-build-test.ps1 in the GDK currently)
	FString DirName;
	switch (MapCategory)
	{
	case EMapCategory::CI_PREMERGE:
		DirName = TEXT("CI_Premerge");
		break;
	case EMapCategory::CI_PREMERGE_SPATIAL_ONLY:
		DirName = TEXT("CI_Premerge_Spatial_Only");
		break;
	case EMapCategory::CI_NIGHTLY:
		DirName = TEXT("CI_Nightly");
		break;
	case EMapCategory::CI_NIGHTLY_SPATIAL_ONLY:
		DirName = TEXT("CI_Nightly_Spatial_Only");
		break;
	case EMapCategory::NO_CI:
		DirName = TEXT("No_CI");
		break;
	default:
		checkNoEntry();
	}

	return GetGeneratedMapFolder() / DirName / MapName + FPackageName::GetMapPackageExtension();
}
