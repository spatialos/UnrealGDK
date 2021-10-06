// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestNetReferenceSettingsOverride.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"

/**
 * This test checks that the test settings overridden in the .ini file have been set correctly
 *
 * Requires SpatialTestNetReferenceMap.ini in \Samples\UnrealGDKTestGyms\Game\Config\MapSettingsOverrides directory with the following
 *values:
 *			[/Script/SpatialGDK.SpatialGDKSettings]
 *			PositionUpdateThresholdMaxCentimeters=1
 */

ASpatialTestNetReferenceSettingsOverride::ASpatialTestNetReferenceSettingsOverride()
	: Super()
{
	Author = "Victoria Bloom";
	Description = TEXT("Test Net Reference - Settings Override");
}

void ASpatialTestNetReferenceSettingsOverride::PrepareTest()
{
	Super::PrepareTest();

	// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected

	AddStep(
		TEXT("Check SpatialGDKSettings override settings"), FWorkerDefinition::AllWorkers, nullptr,
		[this]() {
			float PreviousMaximumDistanceThreshold = GetDefault<USpatialGDKSettings>()->PositionUpdateThresholdMaxCentimeters;
			RequireEqual_Float(PreviousMaximumDistanceThreshold, 0, TEXT("Expected PreviousMaximumDistanceThreshold to equal 1"));

			FinishStep();
		},
		nullptr, 5.0f);

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
}
