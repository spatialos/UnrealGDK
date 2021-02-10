// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventTracingSettingsOverride.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SettingsOverrideHelper.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"
//#include "SpatialGDKEditorSettings.h"

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

	// Check the number of clients

	AddStep(TEXT("Check SpatialGDKSettings override settings"), FWorkerDefinition::AllWorkers, nullptr, [this]() {
		// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected
		bool bEventTracingEnabled = GetDefault<USpatialGDKSettings>()->bEventTracingEnabled;
		RequireTrue(bEventTracingEnabled, TEXT("Expected bEventTracingEnabled to be True"));

		// To verify that the settings get reverted correctly need to run a test on a subsequent map to check these settings have their
		// original values

		FinishStep();
	});

	//	AddStep(TEXT("Check SpatialGDKEditorSettings override settings"), FWorkerDefinition::AllWorkers, nullptr, [this]() {
	//	// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected
	//	FString SpatialOSCommandLineLaunchFlags = GetDefault<USpatialGDKEditorSettings>()->GetSpatialOSCommandLineLaunchFlags();
	//	RequireTrue(SpatialOSCommandLineLaunchFlags.Equals("--event-tracing-enabled=true"), "Expected SpatialOSCommandLineLaunchFlags to
	//contain --event-tracing-enabled=true");

	//	// To verify that the settings get reverted correctly need to run a test on a subsequent map to check these settings have their
	//	// original values

	//	FinishStep();
	//});

	AddStep(TEXT("Check PIE override settings"), FWorkerDefinition::AllServers, nullptr, [this]() {
		// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected

		SettingsOverrideHelper::VerifyNumberOfClients(1, this);

		// To verify that the settings get reverted correctly need to run a test on a subsequent map to check these settings have their
		// original values

		FinishStep();
	});
}
