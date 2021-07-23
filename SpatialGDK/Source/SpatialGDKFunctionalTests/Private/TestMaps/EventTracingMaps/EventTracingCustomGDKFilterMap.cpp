// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/EventTracingMaps/EventTracingCustomGDKFilterMap.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/EventTracingTestGameMode.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/SettingsTests/CustomGDKFilterTest.h"

UEventTracingCustomGDKFilterMap::UEventTracingCustomGDKFilterMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("EventTracingCustomGDKFilterMap"))
{
}

void UEventTracingCustomGDKFilterMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	AddActorToLevel<ACustomGDKFilterTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = AEventTracingTestGameMode::StaticClass();
	WorldSettings->SettingsOverride = {
		"../../../Engine/Plugins/UnrealGDK/SpatialGDK/Config/MapSettingsOverrides/TestOverridesCustomGDKFilter.ini"
	};
}
