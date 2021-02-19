// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionMap.h"

#include "CrossServerMultiPossessionTest.h"
#include "CrossServerPossessionGameMode.h"
#include "CrossServerPossessionLockTest.h"
#include "CrossServerPossessionTest.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "NoneCrossServerPossessionTest.h"
#include "SpatialGDKFunctionalTests/Public/Test2x2WorkerSettings.h"

namespace
{
void SetupPlayerStartAndWorldSettings(UWorld* World)
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Move player start to server 1
	AActor** PlayerStart = CurrentLevel->Actors.FindByPredicate([](AActor* Actor) {
		return Actor->GetClass() == APlayerStart::StaticClass();
	});
	(*PlayerStart)->SetActorLocation(FVector(-300, -300, 100));

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

	SetupPlayerStartAndWorldSettings(World);
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

	SetupPlayerStartAndWorldSettings(World);
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

	SetupPlayerStartAndWorldSettings(World);
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

	SetupPlayerStartAndWorldSettings(World);
}
