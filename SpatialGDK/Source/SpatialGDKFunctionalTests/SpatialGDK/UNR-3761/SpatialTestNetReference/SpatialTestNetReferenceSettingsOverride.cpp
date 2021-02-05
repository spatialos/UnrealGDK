// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestNetReferenceSettingsOverride.h"
#include "SpatialGDKSettings.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "SpatialFunctionalTestFlowController.h"

/**
 * This test checks that the test settings overridden in the .ini file have been set correctly
 *
 * Requires SpatialTestNetReferenceMap.ini in \Samples\UnrealGDKTestGyms\Game\Config directory with the following values:
 *			[/Script/SpatialGDK.SpatialGDKSettings]
 *			PositionUpdateThresholdMaxCentimeters=1
 *
 *			[/Script/UnrealEd.LevelEditorPlaySettings]
*			PlayNumberOfClients=3
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

	// Check the number of clients
	

	AddStep(TEXT("Check SpatialGDKSettings override settings"), FWorkerDefinition::AllWorkers, nullptr, [this]() {

		// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected
		float PreviousMaximumDistanceThreshold = GetDefault<USpatialGDKSettings>()->PositionUpdateThresholdMaxCentimeters;
		RequireTrue(PreviousMaximumDistanceThreshold == 1, "Expected PreviousMaximumDistanceThreshold to equal 1");

		// To verify that the settings get reverted correctly need to run a test on a subsequent map to check these settings have their original
		// values

		FinishStep();
	});

	AddStep(TEXT("Check PIE override settings"), FWorkerDefinition::AllServers, nullptr, [this]() {
		// Settings will have already been automatically overwritten when the map was loaded -> check the settings are as expected
		
		int32 NumberOfClients = 0;
		// Cannot check the ULevelEditorPlaySettings directly as we overwrite a copy - so we can test for the effect instead
		for (ASpatialFunctionalTestFlowController* FlowController : GetFlowControllers())
		{
			if (FlowController->WorkerDefinition.Type == ESpatialFunctionalTestWorkerType::Client)
			{
				NumberOfClients++;
			}
		}
		
		RequireTrue(NumberOfClients == 3, "Expected PlayNumberOfClients to equal 3");

		// To verify that the settings get reverted correctly need to run a test on a subsequent map to check these settings have their
		// original values

		FinishStep();
	});
}
