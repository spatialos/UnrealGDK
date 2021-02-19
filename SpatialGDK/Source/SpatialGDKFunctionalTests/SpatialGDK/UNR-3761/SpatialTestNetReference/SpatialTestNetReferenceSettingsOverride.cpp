// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestNetReferenceSettingsOverride.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKSettings.h"

/**
 * This test checks that the test settings overridden in the .ini file have been set correctly
 *
 * Requires SpatialTestNetReferenceMap.ini in \Samples\UnrealGDKTestGyms\Game\Config\MapSettingsOverrides directory with the following values:
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

	AddStep(TEXT("Check SpatialGDKSettings override settings"), FWorkerDefinition::AllWorkers, nullptr, [this]() {
		float PreviousMaximumDistanceThreshold = GetDefault<USpatialGDKSettings>()->PositionUpdateThresholdMaxCentimeters;
		RequireTrue(PreviousMaximumDistanceThreshold == 0, TEXT("Expected PreviousMaximumDistanceThreshold to equal 1"));

		FinishStep();
	});

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
}
