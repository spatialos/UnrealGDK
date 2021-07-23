// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/EventTracingMaps/EventTracingEventSamplingProbabilityOverrideMap.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/SettingsTests/EventSamplingProbabilityOverrideTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/EventTracingTestGameMode.h"

UEventTracingEventSamplingProbabilityOverrideMap::UEventTracingEventSamplingProbabilityOverrideMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("EventTracingEventSamplingProbabilityOverrideMap"))
{
}

void UEventTracingEventSamplingProbabilityOverrideMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	AddActorToLevel<AEventSamplingProbabilityOverrideTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = AEventTracingTestGameMode::StaticClass();
	WorldSettings->SettingsOverride = {
		"../../../Engine/Plugins/UnrealGDK/SpatialGDK/Config/MapSettingsOverrides/TestOverridesEventSamplingProbabilityOverride.ini"
	};
}
