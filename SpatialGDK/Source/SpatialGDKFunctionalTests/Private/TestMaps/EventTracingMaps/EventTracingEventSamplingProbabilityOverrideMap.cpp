// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/EventTracingMaps/EventTracingEventSamplingProbabilityOverrideMap.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/EventTracingTestGameMode.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/SettingsTests/EventSamplingProbabilityOverrideTest.h"

UEventTracingEventSamplingProbabilityOverrideMap::UEventTracingEventSamplingProbabilityOverrideMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("EventTracingEventSamplingProbabilityOverrideMap"))
{
	SetCustomConfig(TEXT("[/Script/SpatialGDK.SpatialGDKSettings]") LINE_TERMINATOR
		TEXT("bEventTracingEnabled=True") LINE_TERMINATOR
		TEXT("bEventTracingEnabledWithEditor=True") LINE_TERMINATOR
		TEXT("EventTracingSamplingSettingsClass=/Script/SpatialGDKFunctionalTests.EventSamplingProbabilityOverrideSettings"));
}

void UEventTracingEventSamplingProbabilityOverrideMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	AddActorToLevel<AEventSamplingProbabilityOverrideTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = AEventTracingTestGameMode::StaticClass();
}
