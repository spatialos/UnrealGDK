// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionMap.h"

#include "CrossServerMultiPossessionTest.h"
#include "CrossServerPossessionGameMode.h"
#include "CrossServerPossessionLockTest.h"
#include "CrossServerPossessionTest.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "NoneCrossServerPossessionTest.h"
#include "TestWorkerSettings.h"

namespace
{
void SetupWorldSettings(UWorld* World)
{
	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest2x2FullInterestWorkerSettings::StaticClass());
	WorldSettings->DefaultGameMode = ACrossServerPossessionGameMode::StaticClass();
}

FString GetConfigForNumberOfClients(int NumberOfClients)
{
	// clang-format off
	return FString::Printf(TEXT("[/Script/UnrealEd.LevelEditorPlaySettings]") LINE_TERMINATOR
						   TEXT("PlayNumberOfClients=%d"), NumberOfClients);
	// clang-format on
}

} // namespace

UCrossServerPossessionMap::UCrossServerPossessionMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("CrossServerPossessionMap"))
{
	SetCustomConfig(GetConfigForNumberOfClients(1));
}

void UCrossServerPossessionMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the test
	AddActorToLevel<ACrossServerPossessionTest>(CurrentLevel, FTransform::Identity);

	SetupWorldSettings(World);
}

UCrossServerPossessionLockMap::UCrossServerPossessionLockMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("CrossServerPossessionLockMap"))
{
	SetCustomConfig(GetConfigForNumberOfClients(1));
}

void UCrossServerPossessionLockMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the test
	AddActorToLevel<ACrossServerPossessionLockTest>(CurrentLevel, FTransform::Identity);

	SetupWorldSettings(World);
}

UNoneCrossServerPossessionMap::UNoneCrossServerPossessionMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("NoneCrossServerPossessionMap"))
{
	SetCustomConfig(GetConfigForNumberOfClients(1));
}

void UNoneCrossServerPossessionMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the test
	AddActorToLevel<ANoneCrossServerPossessionTest>(CurrentLevel, FTransform::Identity);

	SetupWorldSettings(World);
}

UCrossServerMultiPossessionMap::UCrossServerMultiPossessionMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("CrossServerMultiPossessionMap"))
{
	SetCustomConfig(GetConfigForNumberOfClients(3));
}

void UCrossServerMultiPossessionMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the test
	AddActorToLevel<ACrossServerMultiPossessionTest>(CurrentLevel, FTransform::Identity);

	SetupWorldSettings(World);
}
