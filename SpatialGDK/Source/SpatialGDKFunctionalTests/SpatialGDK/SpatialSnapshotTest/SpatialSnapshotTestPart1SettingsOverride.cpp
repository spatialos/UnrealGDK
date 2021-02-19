// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialSnapshotTestPart1SettingsOverride.h"
#include "Editor/EditorPerformanceSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"

/**
 * This test checks that the test settings overridden in the map .ini file have been set correctly
 *
 * Requires TestOverridesSpatialSnapshotTestPart1Map.ini in \Samples\UnrealGDKTestGyms\Game\Config\MapSettingsOverrides directory with the following values:
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

	// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected

	AddStep(TEXT("Check PIE override settings"), FWorkerDefinition::AllServers, nullptr, [this]() {
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

		FinishStep();
	});

	AddStep(TEXT("Check GeneralProjectSettings override settings"), FWorkerDefinition::AllWorkers, nullptr, [this]() {
		bool bSpatialNetworking = GetDefault<UGeneralProjectSettings>()->UsesSpatialNetworking();
		RequireTrue(bSpatialNetworking, TEXT("Expected bSpatialNetworking to be True"));

		FinishStep();
	});

	AddStep(TEXT("Check Editor Peformance Settings"), FWorkerDefinition::AllServers, nullptr, [this]() {
		bool bThrottleCPUWhenNotForeground = GetDefault<UEditorPerformanceSettings>()->bThrottleCPUWhenNotForeground;
		RequireFalse(bThrottleCPUWhenNotForeground, TEXT("Expected bSpatialNetworking to be False"));

		FinishStep();
	});
}
