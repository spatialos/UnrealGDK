// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/EventTracingMaps/EventTracingZeroSamplingProbabilityFilterTrueMap.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/SettingsTests/ZeroSamplingProbabilityFilterTrueTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMovement/EventTracingTestGameMode.h"

UEventTracingZeroSamplingProbabilityFilterTrueMap::UEventTracingZeroSamplingProbabilityFilterTrueMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("EventTracingZeroSamplingProbabilityFilterTrueMap"))
{
}

void UEventTracingZeroSamplingProbabilityFilterTrueMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	AddActorToLevel<AZeroSamplingProbabilityFilterTrueTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = AEventTracingTestGameMode::StaticClass();
	WorldSettings->SettingsOverride = {
		"../../../Engine/Plugins/UnrealGDK/SpatialGDK/Config/MapSettingsOverrides/TestOverridesZeroSamplingProbabilityFilterTrue.ini"
	};
}
