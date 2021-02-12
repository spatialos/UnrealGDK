// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventTracingSettingsOverride.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"

/**
 * This test checks that the test settings overridden in the .ini file have been set correctly
 *
 * Requires TestOverridesSpatialEventTracingTests.ini in \Samples\UnrealGDKTestGyms\Game\Config directory with the following values:
 *		[/Script/SpatialGDK.SpatialGDKSettings]
 *		bEventTracingEnabled=True
 *
 *		[/Script/SpatialGDKEditor.SpatialGDKEditorSettings]
 *		SpatialOSCommandLineLaunchFlags="--event-tracing-enabled=true"
 *
 *		[/Script/UnrealEd.LevelEditorPlaySettings]
 *		PlayNumberOfClients=1
 */

AEventTracingSettingsOverride::AEventTracingSettingsOverride()
	: Super()
{
	Author = "Victoria Bloom";
	Description = TEXT("Event Tracing - Settings Override");
}

void AEventTracingSettingsOverride::FinishEventTraceTest()
{
	FinishStep();
}

void AEventTracingSettingsOverride::PrepareTest()
{
	Super::PrepareTest();

	// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected

	AddStep(TEXT("Check SpatialGDKSettings override settings"), FWorkerDefinition::AllWorkers, nullptr, [this]() {
		bool bEventTracingEnabled = GetDefault<USpatialGDKSettings>()->bEventTracingEnabled;
		RequireTrue(bEventTracingEnabled, TEXT("Expected bEventTracingEnabled to be True"));

		FinishStep();
	});

	AddStep(TEXT("Check PIE override settings"), FWorkerDefinition::AllServers, nullptr, [this]() {
		int32 ExpectedNumberOfClients = 1;
		int32 RequiredNumberOfClients = GetNumRequiredClients();
		RequireTrue(RequiredNumberOfClients == ExpectedNumberOfClients,
					FString::Printf(TEXT("Expected number of required clients to be %i"), ExpectedNumberOfClients));
		int32 ActualNumberOfClients = GetNumberOfClientWorkers();
		RequireTrue(ActualNumberOfClients == ExpectedNumberOfClients,
					FString::Printf(TEXT("Expected number of actual clients to be %i"), ExpectedNumberOfClients));

		FinishStep();
	});
}
