// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/EventTracingMaps/EventTracingCustomGDKFilterMap.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/EventTracingTestGameMode.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/SettingsTests/CustomGDKFilterTest.h"

UEventTracingCustomGDKFilterMap::UEventTracingCustomGDKFilterMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("EventTracingCustomGDKFilterMap"))
{
	SetCustomConfig(TEXT("[/Script/SpatialGDK.SpatialGDKSettings]") LINE_TERMINATOR
		TEXT("bEventTracingEnabled=True") LINE_TERMINATOR
		TEXT("bEventTracingEnabledWithEditor=True") LINE_TERMINATOR
		TEXT("EventTracingSamplingSettingsClass=/Script/SpatialGDKFunctionalTests.CustomGDKFilterSettings"));
}

void UEventTracingCustomGDKFilterMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	AddActorToLevel<ACustomGDKFilterTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = AEventTracingTestGameMode::StaticClass();
}
