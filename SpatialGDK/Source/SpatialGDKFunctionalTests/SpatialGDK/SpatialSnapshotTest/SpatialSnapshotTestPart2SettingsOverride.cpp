// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialSnapshotTestPart2SettingsOverride.h"
#include "Editor/EditorPerformanceSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"

/**
 * This test checks that the test settings overridden in the .ini file have been set correctly
 *
 * Requires TestOverridesSpatialSnapshotTestPart2Map.ini in \Samples\UnrealGDKTestGyms\Game\Config\MapSettingsOverrides directory with the
 *following values:
 *			[/Script/UnrealEd.LevelEditorPlaySettings]
 *			PlayNumberOfClients=1
 *			PlayNetMode=PIE_Client
 *
 *			[/Script/EngineSettings.GeneralProjectSettings]
 *			bSpatialNetworking=False
 */

ASpatialSnapshotTestPart2SettingsOverride::ASpatialSnapshotTestPart2SettingsOverride()
	: Super()
{
	Author = "Victoria Bloom";
	Description = TEXT("Spatial Snapshot Test Part 2 - Settings Override");
}

void ASpatialSnapshotTestPart2SettingsOverride::PrepareTest()
{
	Super::PrepareTest();

	// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected

	AddStep(
		TEXT("Check PIE override settings"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			int32 ExpectedNumberOfClients = 1;
			int32 RequiredNumberOfClients = GetNumRequiredClients();
			RequireEqual_Int(RequiredNumberOfClients, ExpectedNumberOfClients, TEXT("Expected a certain number of required clients."));
			int32 ActualNumberOfClients = GetNumberOfClientWorkers();
			RequireEqual_Int(ActualNumberOfClients, ExpectedNumberOfClients, TEXT("Expected a certain number of actual clients."));

			const ULevelEditorPlaySettings* EditorPlaySettings = GetDefault<ULevelEditorPlaySettings>();
			EPlayNetMode PlayNetMode = EPlayNetMode::PIE_Standalone;
			EditorPlaySettings->GetPlayNetMode(PlayNetMode);
			RequireTrue(PlayNetMode == EPlayNetMode::PIE_Client, TEXT("Expected the PlayNetMode to be PIE_Client"));

			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Check GeneralProjectSettings override settings"), FWorkerDefinition::AllWorkers, nullptr,
		[this]() {
			bool bSpatialNetworking = GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
			RequireFalse(bSpatialNetworking, TEXT("Expected bSpatialNetworking to be False"));

			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Check Editor Peformance Settings"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			bool bThrottleCPUWhenNotForeground = GetDefault<UEditorPerformanceSettings>()->bThrottleCPUWhenNotForeground;
			RequireTrue(bThrottleCPUWhenNotForeground, TEXT("Expected bSpatialNetworking to be True"));

			FinishStep();
		},
		nullptr, 5.0f);
}
