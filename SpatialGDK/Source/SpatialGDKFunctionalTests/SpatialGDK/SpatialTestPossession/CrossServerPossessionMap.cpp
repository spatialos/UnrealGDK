// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CrossServerPossessionMap.h"

#include "CrossServerMultiPossessionTest.h"
#include "CrossServerPossessionGameMode.h"
#include "CrossServerPossessionLockTest.h"
#include "CrossServerPossessionTest.h"
#include "EngineClasses/SpatialWorldSettings.h"
#include "NoneCrossServerPossessionTest.h"
#include "TestWorkerSettings.h"

namespace CrossServerPossessionMapPrivate
{
void SetupWorldSettings(UWorld& World)
{
	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World.GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest2x2FullInterestWorkerSettings::StaticClass());
	WorldSettings->DefaultGameMode = ACrossServerPossessionGameMode::StaticClass();
}

} // namespace CrossServerPossessionMapPrivate

UCrossServerPossessionMap::UCrossServerPossessionMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("CrossServerPossessionMap"))
{
	SetNumberOfClients(1);
}

void UCrossServerPossessionMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the tests
	AddActorToLevel<ACrossServerPossessionTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ACrossServerPossessionLockTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<ANoneCrossServerPossessionTest>(CurrentLevel, FTransform::Identity);

	CrossServerPossessionMapPrivate::SetupWorldSettings(*World);
}

UCrossServerMultiPossessionMap::UCrossServerMultiPossessionMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("CrossServerMultiPossessionMap"))
{
	SetNumberOfClients(3);
}

void UCrossServerMultiPossessionMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	// Add the test
	AddActorToLevel<ACrossServerMultiPossessionTest>(CurrentLevel, FTransform::Identity);

	CrossServerPossessionMapPrivate::SetupWorldSettings(*World);
}
