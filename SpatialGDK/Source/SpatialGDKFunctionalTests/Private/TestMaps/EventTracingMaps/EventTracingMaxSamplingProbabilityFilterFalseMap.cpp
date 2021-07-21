// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/EventTracingMaps/EventTracingMaxSamplingProbabilityFilterFalseMap.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/SettingsTests/MaxSamplingProbabilityFilterFalseTest.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/SpatialTestCharacterMovement/EventTracingTestGameMode.h"

UEventTracingMaxSamplingProbabilityFilterFalseMap::UEventTracingMaxSamplingProbabilityFilterFalseMap()
	: UGeneratedTestMap(EMapCategory::CI_NIGHTLY_SPATIAL_ONLY, TEXT("EventTracingMaxSamplingProbabilityFilterFalseMap"))
{
}

void UEventTracingMaxSamplingProbabilityFilterFalseMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	AddActorToLevel<AMaxSamplingProbabilityFilterFalseTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = AEventTracingTestGameMode::StaticClass();
	WorldSettings->SettingsOverride = { "../../../Engine/Plugins/UnrealGDK/SpatialGDK/Config/MapSettingsOverrides/TestOverridesMaxSamplingProbabilityFilterFalse.ini" };
}
