// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventTracingSettingsOverride.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"

/**
 * This test checks that the test settings overridden in the .ini file have been set correctly
 *
 * Requires TestOverridesSpatialEventTracingTests.ini in \Samples\UnrealGDKTestGyms\Game\Config\MapSettingsOverrides directory with the
 *following values:
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

	AddStep(
		TEXT("Check SpatialGDKSettings override settings"), FWorkerDefinition::AllWorkers, nullptr,
		[this]() {
			bool bEventTracingEnabled = GetDefault<USpatialGDKSettings>()->bEventTracingEnabled;
			RequireTrue(bEventTracingEnabled, TEXT("Expected bEventTracingEnabled to be True"));

			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Check PIE override settings"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			int32 ExpectedNumberOfClients = 1;
			int32 RequiredNumberOfClients = GetNumRequiredClients();
			RequireEqual_Int(RequiredNumberOfClients, ExpectedNumberOfClients, TEXT("Expected a certain number of required clients."));
			int32 ActualNumberOfClients = GetNumberOfClientWorkers();
			RequireEqual_Int(ActualNumberOfClients, ExpectedNumberOfClients, TEXT("Expected a certain number of actual clients."));

			FinishStep();
		},
		nullptr, 5.0f);
}
