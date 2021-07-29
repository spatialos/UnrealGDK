// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestMaps/EventTracingMaps/EventTracingZeroSamplingProbabilityFilterTrueMap.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/EventTracingTestGameMode.h"
#include "SpatialGDKFunctionalTests/SpatialGDK/EventTracingTests/SettingsTests/ZeroSamplingProbabilityFilterTrueTest.h"

UEventTracingZeroSamplingProbabilityFilterTrueMap::UEventTracingZeroSamplingProbabilityFilterTrueMap()
	: UGeneratedTestMap(EMapCategory::NO_CI, TEXT("EventTracingZeroSamplingProbabilityFilterTrueMap"))
{
	SetCustomConfig(
		TEXT("[/Script/SpatialGDK.SpatialGDKSettings]") LINE_TERMINATOR TEXT("bEventTracingEnabled=True")
			LINE_TERMINATOR TEXT("bEventTracingEnabledWithEditor=True") LINE_TERMINATOR TEXT(
				"EventTracingSamplingSettingsClass=/Script/SpatialGDKFunctionalTests.ZeroSamplingProbabilityFilterTrueSettings"));
}

void UEventTracingZeroSamplingProbabilityFilterTrueMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	AddActorToLevel<AZeroSamplingProbabilityFilterTrueTest>(CurrentLevel, FTransform::Identity);

	ASpatialWorldSettings* WorldSettings = Cast<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->DefaultGameMode = AEventTracingTestGameMode::StaticClass();
}
