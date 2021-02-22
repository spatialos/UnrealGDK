// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionMap.h"

#include "CrossServerMultiPossessionTest.h"
#include "CrossServerPossessionGameMode.h"
#include "CrossServerPossessionLockTest.h"
#include "CrossServerPossessionTest.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "NoneCrossServerPossessionTest.h"
#include "SpatialGDKFunctionalTests/Public/Test2x2WorkerSettings.h"

namespace
{
void SetupWorldSettings(UWorld* World)
{
	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest2x2WorkerSettings::StaticClass());
	WorldSettings->DefaultGameMode = ACrossServerPossessionGameMode::StaticClass();
	WorldSettings->TestingSettings.TestingMode = EMapTestingMode::Detect;
}

} // namespace

UCrossServerPossessionMap::UCrossServerPossessionMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("CrossServerPossessionMap"))
{
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
}

void UCrossServerMultiPossessionMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the test
	AddActorToLevel<ACrossServerMultiPossessionTest>(CurrentLevel, FTransform::Identity);

	SetupWorldSettings(World);
}
