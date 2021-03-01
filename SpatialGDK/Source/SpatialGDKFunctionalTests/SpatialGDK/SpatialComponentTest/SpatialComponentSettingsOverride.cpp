// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialComponentSettingsOverride.h"
#include "Editor/EditorPerformanceSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"

/**
 * This test checks that the test settings overridden in the base.ini file have been set correctly, this map specifically does not have its
 * own overrides.
 *
 * Requires TestOverridesBase.ini in \Samples\UnrealGDKTestGyms\Game\Config directory with the following values:
 *		[/Script/UnrealEd.LevelEditorPlaySettings]
 *		PlayNumberOfClients=2
 *
 *		[/Script/UnrealEd.EditorPerformanceSettings]
 *		bThrottleCPUWhenNotForeground=False
 */

ASpatialComponentSettingsOverride::ASpatialComponentSettingsOverride()
	: Super()
{
	Author = "Victoria Bloom";
	Description = TEXT("Test Base - Settings Override");
}

void ASpatialComponentSettingsOverride::PrepareTest()
{
	Super::PrepareTest();
	// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected

	AddStep(
		TEXT("Check PIE override settings"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			int32 ExpectedNumberOfClients = 2;
			int32 RequiredNumberOfClients = GetNumRequiredClients();
			RequireEqual_Int(RequiredNumberOfClients, ExpectedNumberOfClients, TEXT("Expected a certain number of required clients."));
			int32 ActualNumberOfClients = GetNumberOfClientWorkers();
			RequireEqual_Int(ActualNumberOfClients, ExpectedNumberOfClients, TEXT("Expected a certain number of actual clients."));

			FinishStep();
		},
		nullptr, 5.0f);

	AddStep(
		TEXT("Check Editor Peformance Settings"), FWorkerDefinition::AllServers, nullptr,
		[this]() {
			bool bThrottleCPUWhenNotForeground = GetDefault<UEditorPerformanceSettings>()->bThrottleCPUWhenNotForeground;
			RequireFalse(bThrottleCPUWhenNotForeground, TEXT("Expected bThrottleCPUWhenNotForeground to be False"));

			FinishStep();
		},
		nullptr, 5.0f);
}
