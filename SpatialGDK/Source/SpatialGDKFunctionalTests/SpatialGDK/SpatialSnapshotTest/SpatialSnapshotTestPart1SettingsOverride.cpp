// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialSnapshotTestPart1SettingsOverride.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"
#include "Editor/EditorPerformanceSettings.h"

/**
 * This test checks that the test settings overridden in the .ini file have been set correctly
 *
 * Requires TestOverridesSpatialSnapshotTestPart1Map.ini in \Samples\UnrealGDKTestGyms\Game\Config directory with the following values:
 *			[/Script/UnrealEd.LevelEditorPlaySettings]
 *			PlayNumberOfClients=1
 *			PlayNetMode=PIE_Client
 *
 *			[/Script/EngineSettings.GeneralProjectSettings]
 *			bSpatialNetworking=True
 */

ASpatialSnapshotTestPart1SettingsOverride::ASpatialSnapshotTestPart1SettingsOverride()
	: Super()
{
	Author = "Victoria Bloom";
	Description = TEXT("Spatial Snapshot Test Part 1 - Settings Override");
}

void ASpatialSnapshotTestPart1SettingsOverride::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Check PIE override settings"), FWorkerDefinition::AllServers, nullptr, [this]() {
		// Check for default number of clients when setting is not overriden
		// Override the LevelEditorPlaySettings in a copy so cannot check the original here, instead check the number of clients actually
		// running
		int32 ExpectedNumberOfClients = 1;
		int32 RequiredNumberOfClients = GetNumRequiredClients();
		RequireTrue(RequiredNumberOfClients == ExpectedNumberOfClients,
					FString::Printf(TEXT("Expected number of required clients to be %i"), ExpectedNumberOfClients));
		int32 ActualNumberOfClients = GetNumberOfClientWorkers();
		RequireTrue(ActualNumberOfClients == ExpectedNumberOfClients,
					FString::Printf(TEXT("Expected number of actual clients to be %i"), ExpectedNumberOfClients));

		const ULevelEditorPlaySettings* EditorPlaySettings = GetDefault<ULevelEditorPlaySettings>();
		EPlayNetMode PlayNetMode = EPlayNetMode::PIE_Standalone;
		EditorPlaySettings->GetPlayNetMode(PlayNetMode);
		RequireTrue(PlayNetMode == EPlayNetMode::PIE_Client, TEXT("Expected the PlayNetMode to be PIE_Client"));

		// To verify that the settings get reverted correctly need to run a test on a subsequent map to check these settings have their
		// original values

		FinishStep();
	});

	AddStep(TEXT("Check GeneralProjectSettings override settings"), FWorkerDefinition::AllWorkers, nullptr, [this]() {
		// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected
		bool bSpatialNetworking = GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
		RequireTrue(bSpatialNetworking, TEXT("Expected bSpatialNetworking to be True"));

		// To verify that the settings get reverted correctly need to run a test on a subsequent map to check these settings have their
		// original values

		FinishStep();
	});

	AddStep(TEXT("Check Editor Peformance Settings"), FWorkerDefinition::AllServers, nullptr, [this]() {
		// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected
		bool bThrottleCPUWhenNotForeground = GetDefault<UEditorPerformanceSettings>()->bThrottleCPUWhenNotForeground;
		RequireFalse(bThrottleCPUWhenNotForeground, TEXT("Expected bSpatialNetworking to be False"));
		// To verify that the settings get reverted correctly need to run a test on a subsequent map to check these settings have their
		// original values

		FinishStep();
	});
}
