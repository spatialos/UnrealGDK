// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/EventTracingMaps/EventTracingMaxSamplingProbabilityFilterFalseMap.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/EventTracingTestGameMode.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/SettingsTests/MaxSamplingProbabilityFilterFalseTest.h"

UEventTracingMaxSamplingProbabilityFilterFalseMap::UEventTracingMaxSamplingProbabilityFilterFalseMap()
	: UGeneratedTestMap(EMapCategory::NO_CI, TEXT("EventTracingMaxSamplingProbabilityFilterFalseMap"))
{
	SetCustomConfig(
		TEXT("[/Script/SpatialGDK.SpatialGDKSettings]") LINE_TERMINATOR TEXT("bEventTracingEnabled=True")
			LINE_TERMINATOR TEXT("bEventTracingEnabledWithEditor=True") LINE_TERMINATOR TEXT(
				"EventTracingSamplingSettingsClass=/Script/SpatialGDKFunctionalTests.MaxSamplingProbabilityFilterFalseSettings"));
}

void UEventTracingMaxSamplingProbabilityFilterFalseMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	AddActorToLevel<AMaxSamplingProbabilityFilterFalseTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = AEventTracingTestGameMode::StaticClass();
}
