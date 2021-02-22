// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialAuthoritySettingsOverride.h"
#include "Editor/EditorPerformanceSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"

/**
 * This test checks that the test settings overridden in a generated map have been set correctly, generated config files are stored in a
 * different location.
 *
 * Requires TestOverridesSpatialAuthorityMap.ini in \Samples\UnrealGDKTestGyms\Game\Intermediate\Config\MapSettingsOverrides directory with
 * the following values:
 *		[/Script/UnrealEd.LevelEditorPlaySettings]
 *		PlayNumberOfClients=1
 */

ASpatialAuthoritySettingsOverride::ASpatialAuthoritySettingsOverride()
	: Super()
{
	Author = "Victoria Bloom";
	Description = TEXT("Spatial Authority Generated Map Test - Settings Override");
}

void ASpatialAuthoritySettingsOverride::PrepareTest()
{
	Super::PrepareTest();
	// Settings will have already been automatically overwritten when the generated map was loaded -> check the settings are as expected

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
