// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialComponentSettingsOverride.h"
#include "Editor/EditorPerformanceSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"

/**
 * This test checks that the test settings overridden in the base.ini file have been set correctly, this map specifically does not have its
 *own overrides.
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

	AddStep(TEXT("Check PIE override settings"), FWorkerDefinition::AllServers, nullptr, [this]() {
		int32 ExpectedNumberOfClients = 2;
		int32 RequiredNumberOfClients = GetNumRequiredClients();
		RequireTrue(RequiredNumberOfClients == ExpectedNumberOfClients,
					FString::Printf(TEXT("Expected number of required clients to be %i"), ExpectedNumberOfClients));
		int32 ActualNumberOfClients = GetNumberOfClientWorkers();
		RequireTrue(ActualNumberOfClients == ExpectedNumberOfClients,
					FString::Printf(TEXT("Expected number of actual clients to be %i"), ExpectedNumberOfClients));

		FinishStep();
	});

	AddStep(TEXT("Check Editor Peformance Settings"), FWorkerDefinition::AllServers, nullptr, [this]() {
		bool bThrottleCPUWhenNotForeground = GetDefault<UEditorPerformanceSettings>()->bThrottleCPUWhenNotForeground;
		RequireFalse(bThrottleCPUWhenNotForeground, TEXT("Expected bSpatialNetworking to be False"));

		FinishStep();
	});
}
